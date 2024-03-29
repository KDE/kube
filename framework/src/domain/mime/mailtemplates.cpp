/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>
    Copyright (c) 2017 Christian Mollekopf <mollekopf@kolabsys.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#include "mailtemplates.h"

#include <functional>
#include <QByteArray>
#include <QList>
#include <QDebug>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QSysInfo>
#include <QUuid>
#include <QTextCodec>
#include <QTextDocument>

#include <KCodecs/KCharsets>
#include <KMime/Types>

#include <sink/mimetreeparser/objecttreeparser.h>

#include "mailcrypto.h"

QDebug operator<<(QDebug dbg, const KMime::Types::Mailbox &mb)
{
    dbg << mb.addrSpec().asString();
    return dbg;
}

namespace KMime {
    namespace Types {
static bool operator==(const KMime::Types::AddrSpec &left, const KMime::Types::AddrSpec &right)
{
    return (left.asString() == right.asString());
}

static bool operator==(const KMime::Types::Mailbox &left, const KMime::Types::Mailbox &right)
{
    return (left.addrSpec().asString() == right.addrSpec().asString());
}
    }

    Message* contentToMessage(Content* content) {
        content->assemble();
        const auto encoded = content->encodedContent();

        auto message = new Message();
        message->setContent(encoded);
        message->parse();

        return message;
    }

}

static KMime::Types::Mailbox::List stripMyAddressesFromAddressList(const KMime::Types::Mailbox::List &list, const KMime::Types::AddrSpecList me)
{
    KMime::Types::Mailbox::List addresses(list);
    for (KMime::Types::Mailbox::List::Iterator it = addresses.begin(); it != addresses.end();) {
        if (me.contains(it->addrSpec())) {
            it = addresses.erase(it);
        } else {
            ++it;
        }
    }

    return addresses;
}

static QString toPlainText(const QString &s)
{
    QTextDocument doc;
    doc.setHtml(s);
    return doc.toPlainText();
}

QString replacePrefixes(const QString &str, const QStringList &prefixRegExps, const QString &newPrefix)
{
    // construct a big regexp that
    // 1. is anchored to the beginning of str (sans whitespace)
    // 2. matches at least one of the part regexps in prefixRegExps
    const QString bigRegExp = QStringLiteral("^(?:\\s+|(?:%1))+\\s*").arg(prefixRegExps.join(QStringLiteral(")|(?:")));
    QRegExp rx(bigRegExp, Qt::CaseInsensitive);
    if (!rx.isValid()) {
        qWarning() << "bigRegExp = \""
                    << bigRegExp << "\"\n"
                    << "prefix regexp is invalid!";
        qWarning() << "Error: " << rx.errorString() << rx;
        Q_ASSERT(false);
        return str;
    }

    QString tmp = str;
    //We expect a match at the beginning of the string
    if (rx.indexIn(tmp) == 0) {
        return tmp.replace(0, rx.matchedLength(), newPrefix + QLatin1String(" "));
    }
    //No match, we just prefix the newPrefix
    return newPrefix + " " + str;
}

const QStringList getForwardPrefixes()
{
    //See https://en.wikipedia.org/wiki/List_of_email_subject_abbreviations
    QStringList list;
    //We want to be able to potentially reply to a variety of languages, so only translating is not enough
    list << QObject::tr("fwd");
    list << "fwd";
    list << "fw";
    list << "wg";
    list << "vs";
    list << "tr";
    list << "rv";
    list << "enc";
    return list;
}


static QString forwardSubject(const QString &s)
{
    //The standandard prefix
    const auto localPrefix = "FW:";
    QStringList forwardPrefixes;
    for (const auto &prefix : getForwardPrefixes()) {
        forwardPrefixes << prefix + "\\s*:";
    }
    return replacePrefixes(s, forwardPrefixes, localPrefix);
}

static QStringList getReplyPrefixes()
{
    //See https://en.wikipedia.org/wiki/List_of_email_subject_abbreviations
    QStringList list;
    //We want to be able to potentially reply to a variety of languages, so only translating is not enough
    list << QObject::tr("re");
    list << "re";
    list << "aw";
    list << "sv";
    list << "antw";
    list << "ref";
    return list;
}

static QString replySubject(const QString &s)
{
    //The standandard prefix (latin for "in re", in matter of)
    const auto localPrefix = "RE:";
    QStringList replyPrefixes;
    for (const auto &prefix : getReplyPrefixes()) {
        replyPrefixes << prefix + "\\s*:";
        replyPrefixes << prefix + "\\[.+\\]:";
        replyPrefixes << prefix + "\\d+:";
    }
    return replacePrefixes(s, replyPrefixes, localPrefix);
}

static QByteArray getRefStr(const QByteArray &references, const QByteArray &messageId)
{
    QByteArray firstRef, lastRef, refStr{references.trimmed()}, retRefStr;
    int i, j;

    if (refStr.isEmpty()) {
        return messageId;
    }

    i = refStr.indexOf('<');
    j = refStr.indexOf('>');
    firstRef = refStr.mid(i, j - i + 1);
    if (!firstRef.isEmpty()) {
        retRefStr = firstRef + ' ';
    }

    i = refStr.lastIndexOf('<');
    j = refStr.lastIndexOf('>');

    lastRef = refStr.mid(i, j - i + 1);
    if (!lastRef.isEmpty() && lastRef != firstRef) {
        retRefStr += lastRef + ' ';
    }

    retRefStr += messageId;
    return retRefStr;
}

KMime::Content *createPlainPartContent(const QString &plainBody, KMime::Content *parent = nullptr)
{
    KMime::Content *textPart = new KMime::Content(parent);
    textPart->contentType()->setMimeType("text/plain");
    //FIXME This is supposed to select a charset out of the available charsets that contains all necessary characters to render the text
    // QTextCodec *charset = selectCharset(m_charsets, plainBody);
    // textPart->contentType()->setCharset(charset->name());
    textPart->contentType()->setCharset("utf-8");
    textPart->contentTransferEncoding()->setEncoding(KMime::Headers::CE8Bit);
    textPart->fromUnicodeString(plainBody);
    return textPart;
}

KMime::Content *createMultipartAlternativeContent(const QString &plainBody, const QString &htmlBody, KMime::Message *parent = nullptr)
{
    KMime::Content *multipartAlternative = new KMime::Content(parent);
    multipartAlternative->contentType()->setMimeType("multipart/alternative");
    multipartAlternative->contentType()->setBoundary(KMime::multiPartBoundary());

    KMime::Content *textPart = createPlainPartContent(plainBody, multipartAlternative);
    multipartAlternative->addContent(textPart);

    KMime::Content *htmlPart = new KMime::Content(multipartAlternative);
    htmlPart->contentType()->setMimeType("text/html");
    //FIXME This is supposed to select a charset out of the available charsets that contains all necessary characters to render the text
    // QTextCodec *charset = selectCharset(m_charsets, htmlBody);
    // htmlPart->contentType()->setCharset(charset->name());
    htmlPart->contentType()->setCharset("utf-8");
    htmlPart->contentTransferEncoding()->setEncoding(KMime::Headers::CE8Bit);
    htmlPart->fromUnicodeString(htmlBody);
    multipartAlternative->addContent(htmlPart);

    return multipartAlternative;
}

KMime::Content *createMultipartMixedContent(QVector<KMime::Content *> contents)
{
    KMime::Content *multiPartMixed = new KMime::Content();
    multiPartMixed->contentType()->setMimeType("multipart/mixed");
    multiPartMixed->contentType()->setBoundary(KMime::multiPartBoundary());

    for (const auto &content : contents) {
        multiPartMixed->addContent(content);
    }

    return multiPartMixed;
}

QString plainToHtml(const QString &body)
{
    QString str = body;
    str = str.toHtmlEscaped();
    str.replace(QStringLiteral("\n"), QStringLiteral("<br />\n"));
    return str;
}

//TODO implement this function using a DOM tree parser
void makeValidHtml(QString &body, const QString &headElement)
{
    QRegExp regEx;
    regEx.setMinimal(true);
    regEx.setPattern(QStringLiteral("<html.*>"));

    if (!body.isEmpty() && !body.contains(regEx)) {
        regEx.setPattern(QStringLiteral("<body.*>"));
        if (!body.contains(regEx)) {
            body = QLatin1String("<body>") + body + QLatin1String("<br/></body>");
        }
        regEx.setPattern(QStringLiteral("<head.*>"));
        if (!body.contains(regEx)) {
            body = QLatin1String("<head>") + headElement + QLatin1String("</head>") + body;
        }
        body = QLatin1String("<html>") + body + QLatin1String("</html>");
    }
}

//FIXME strip signature works partially for HTML mails
static QString stripSignature(const QString &msg)
{
    // Following RFC 3676, only > before --
    // I prefer to not delete a SB instead of delete good mail content.
    // We expect no CRLF from the ObjectTreeParser. The regex won't handle it.
    if (msg.contains("\r\n")) {
        qWarning() << "Message contains CRLF, but shouldn't: " << msg;
        Q_ASSERT(false);
    }
    const QRegExp sbDelimiterSearch = QRegExp(QLatin1String("(^|\n)[> ]*-- \n"));
    // The regular expression to look for prefix change
    const QRegExp commonReplySearch = QRegExp(QLatin1String("^[ ]*>"));

    QString res = msg;
    int posDeletingStart = 1; // to start looking at 0

    // While there are SB delimiters (start looking just before the deleted SB)
    while ((posDeletingStart = res.indexOf(sbDelimiterSearch, posDeletingStart - 1)) >= 0) {
        QString prefix; // the current prefix
        QString line; // the line to check if is part of the SB
        int posNewLine = -1;

        // Look for the SB beginning
        int posSignatureBlock = res.indexOf(QLatin1Char('-'), posDeletingStart);
        // The prefix before "-- "$
        if (res.at(posDeletingStart) == QLatin1Char('\n')) {
            ++posDeletingStart;
        }

        prefix = res.mid(posDeletingStart, posSignatureBlock - posDeletingStart);
        posNewLine = res.indexOf(QLatin1Char('\n'), posSignatureBlock) + 1;

        // now go to the end of the SB
        while (posNewLine < res.size() && posNewLine > 0) {
            // handle the undefined case for mid ( x , -n ) where n>1
            int nextPosNewLine = res.indexOf(QLatin1Char('\n'), posNewLine);

            if (nextPosNewLine < 0) {
                nextPosNewLine = posNewLine - 1;
            }

            line = res.mid(posNewLine, nextPosNewLine - posNewLine);

            // check when the SB ends:
            // * does not starts with prefix or
            // * starts with prefix+(any substring of prefix)
            if ((prefix.isEmpty() && line.indexOf(commonReplySearch) < 0) ||
                    (!prefix.isEmpty() && line.startsWith(prefix) &&
                     line.mid(prefix.size()).indexOf(commonReplySearch) < 0)) {
                posNewLine = res.indexOf(QLatin1Char('\n'), posNewLine) + 1;
            } else {
                break;    // end of the SB
            }
        }

        // remove the SB or truncate when is the last SB
        if (posNewLine > 0) {
            res.remove(posDeletingStart, posNewLine - posDeletingStart);
        } else {
            res.truncate(posDeletingStart);
        }
    }

    return res;
}

static void setupPage(QWebEnginePage *page)
{
        page->profile()->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
        page->profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
        page->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
        page->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
        page->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::XSSAuditingEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
        page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
        page->settings()->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
        page->settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
}

static void plainMessageText(const QString &plainTextContent, const QString &htmlContent, const std::function<void(const QString &)> &callback)
{
    const auto result = plainTextContent.isEmpty() ? toPlainText(htmlContent) : plainTextContent;
    callback(result);
}

static QString extractHeaderBodyScript()
{
    return QStringLiteral("(function() {"
                          "var res = {"
                          "    body: document.getElementsByTagName('body')[0].innerHTML,"
                          "    header: document.getElementsByTagName('head')[0].innerHTML"
                          "};"
                          "return res;"
                          "})()");
}

void htmlMessageText(const QString &plainTextContent, const QString &htmlContent, const std::function<void(const QString &body, QString &head)> &callback)
{
    QString htmlElement = htmlContent;

    if (htmlElement.isEmpty()) {   //plain mails only
        QString htmlReplace = plainTextContent.toHtmlEscaped();
        htmlReplace = htmlReplace.replace(QStringLiteral("\n"), QStringLiteral("<br />"));
        htmlElement = QStringLiteral("<html><head></head><body>%1</body></html>\n").arg(htmlReplace);
    }

    auto page = new QWebEnginePage;
    setupPage(page);

    page->setHtml(htmlElement);
    page->runJavaScript(extractHeaderBodyScript(), QWebEngineScript::ApplicationWorld, [=](const QVariant &result){
        page->deleteLater();
        const QVariantMap map = result.toMap();
        auto bodyElement = map.value(QStringLiteral("body")).toString();
        auto headerElement = map.value(QStringLiteral("header")).toString();
        if (!bodyElement.isEmpty()) {
            return callback(bodyElement, headerElement);
        }

        return callback(htmlElement, headerElement);
    });
}

QString formatQuotePrefix(const QString &wildString, const QString &fromDisplayString)
{
    QString result;

    if (wildString.isEmpty()) {
        return wildString;
    }

    unsigned int strLength(wildString.length());
    for (uint i = 0; i < strLength;) {
        QChar ch = wildString[i++];
        if (ch == QLatin1Char('%') && i < strLength) {
            ch = wildString[i++];
            switch (ch.toLatin1()) {
            case 'f': { // sender's initals
                if (fromDisplayString.isEmpty()) {
                    break;
                }

                uint j = 0;
                const unsigned int strLength(fromDisplayString.length());
                for (; j < strLength && fromDisplayString[j] > QLatin1Char(' '); ++j)
                    ;
                for (; j < strLength && fromDisplayString[j] <= QLatin1Char(' '); ++j)
                    ;
                result += fromDisplayString[0];
                if (j < strLength && fromDisplayString[j] > QLatin1Char(' ')) {
                    result += fromDisplayString[j];
                } else if (strLength > 1) {
                    if (fromDisplayString[1] > QLatin1Char(' ')) {
                        result += fromDisplayString[1];
                    }
                }
            }
            break;
            case '_':
                result += QLatin1Char(' ');
                break;
            case '%':
                result += QLatin1Char('%');
                break;
            default:
                result += QLatin1Char('%');
                result += ch;
                break;
            }
        } else {
            result += ch;
        }
    }
    return result;
}

QString quotedPlainText(const QString &selection, const QString &fromDisplayString)
{
    QString content = selection;
    // Remove blank lines at the beginning:
    const int firstNonWS = content.indexOf(QRegExp(QLatin1String("\\S")));
    const int lineStart = content.lastIndexOf(QLatin1Char('\n'), firstNonWS);
    if (lineStart >= 0) {
        content.remove(0, static_cast<unsigned int>(lineStart));
    }

    const auto quoteString = QStringLiteral("> ");
    const QString indentStr = formatQuotePrefix(quoteString, fromDisplayString);
    //FIXME
    // if (TemplateParserSettings::self()->smartQuote() && mWrap) {
    //     content = MessageCore::StringUtil::smartQuote(content, mColWrap - indentStr.length());
    // }
    content.replace(QLatin1Char('\n'), QLatin1Char('\n') + indentStr);
    content.prepend(indentStr);
    content += QLatin1Char('\n');

    return content;
}

QString quotedHtmlText(const QString &selection)
{
    QString content = selection;
    //TODO 1) look for all the variations of <br>  and remove the blank lines
    //2) implement vertical bar for quoted HTML mail.
    //3) After vertical bar is implemented, If a user wants to edit quoted message,
    // then the <blockquote> tags below should open and close as when required.

    //Add blockquote tag, so that quoted message can be differentiated from normal message
    content = QLatin1String("<blockquote>") + content + QLatin1String("</blockquote>");
    return content;
}

void applyCharset(const KMime::Message::Ptr msg, const KMime::Message::Ptr &origMsg)
{
    // first convert the body from its current encoding to unicode representation
    QTextCodec *bodyCodec = KCharsets::charsets()->codecForName(QString::fromLatin1(msg->contentType()->charset()));
    if (!bodyCodec) {
        bodyCodec = KCharsets::charsets()->codecForName(QStringLiteral("UTF-8"));
    }

    const QString body = bodyCodec->toUnicode(msg->body());

    // then apply the encoding of the original message
    msg->contentType()->setCharset(origMsg->contentType()->charset());

    QTextCodec *codec = KCharsets::charsets()->codecForName(QString::fromLatin1(msg->contentType()->charset()));
    if (!codec) {
        qCritical() << "Could not get text codec for charset" << msg->contentType()->charset();
    } else if (!codec->canEncode(body)) {     // charset can't encode body, fall back to preferred
        const QStringList charsets /*= preferredCharsets() */;

        QList<QByteArray> chars;
        chars.reserve(charsets.count());
        foreach (const QString &charset, charsets) {
            chars << charset.toLatin1();
        }

        //FIXME
        QByteArray fallbackCharset/* = selectCharset(chars, body)*/;
        if (fallbackCharset.isEmpty()) { // UTF-8 as fall-through
            fallbackCharset = "UTF-8";
        }

        codec = KCharsets::charsets()->codecForName(QString::fromLatin1(fallbackCharset));
        msg->setBody(codec->fromUnicode(body));
    } else {
        msg->setBody(codec->fromUnicode(body));
    }
}

enum ReplyStrategy {
    ReplyList,
    ReplySmart,
    ReplyAll,
    ReplyAuthor,
    ReplyNone
};


static QByteArray as7BitString(const KMime::Headers::Base *h)
{
    if (h) {
        return h->as7BitString(false);
    }
    return {};
}


static QString asUnicodeString(const KMime::Headers::Base *h)
{
    if (h) {
        return h->asUnicodeString();
    }
    return {};
}

static KMime::Types::Mailbox::List getMailingListAddresses(const KMime::Headers::Base *listPostHeader)
{
    KMime::Types::Mailbox::List mailingListAddresses;
    const QString listPost = asUnicodeString(listPostHeader);
    if (listPost.contains(QStringLiteral("mailto:"), Qt::CaseInsensitive)) {
        QRegExp rx(QStringLiteral("<mailto:([^@>]+)@([^>]+)>"), Qt::CaseInsensitive);
        if (rx.indexIn(listPost, 0) != -1) {   // matched
            KMime::Types::Mailbox mailbox;
            mailbox.fromUnicodeString(rx.cap(1) + QLatin1Char('@') + rx.cap(2));
            mailingListAddresses << mailbox;
        }
    }
    return mailingListAddresses;
}

struct RecipientMailboxes {
    KMime::Types::Mailbox::List to;
    KMime::Types::Mailbox::List cc;
};

static RecipientMailboxes getRecipients(const KMime::Types::Mailbox::List &from, const KMime::Types::Mailbox::List &to, const KMime::Types::Mailbox::List &cc, const KMime::Types::Mailbox::List &replyToList, const KMime::Types::Mailbox::List &mailingListAddresses, const KMime::Types::AddrSpecList &me)
{
    KMime::Types::Mailbox::List toList;
    KMime::Types::Mailbox::List ccList;
    auto listContainsMe = [&] (const KMime::Types::Mailbox::List &list) {
        for (const auto &m : me) {
            KMime::Types::Mailbox mailbox;
            mailbox.setAddress(m);
            if (list.contains(mailbox)) {
                return true;
            }
        }
        return false;
    };

    if (listContainsMe(from)) {
        // sender seems to be one of our own identities, so we assume that this
        // is a reply to a "sent" mail where the users wants to add additional
        // information for the recipient.
        return {to, cc};
    }

    KMime::Types::Mailbox::List recipients;
    KMime::Types::Mailbox::List ccRecipients;

    // add addresses from the Reply-To header to the list of recipients
    if (!replyToList.isEmpty()) {
        recipients = replyToList;

        // strip all possible mailing list addresses from the list of Reply-To addresses
        foreach (const KMime::Types::Mailbox &mailbox, mailingListAddresses) {
            foreach (const KMime::Types::Mailbox &recipient, recipients) {
                if (mailbox == recipient) {
                    recipients.removeAll(recipient);
                }
            }
        }
    }

    if (!mailingListAddresses.isEmpty()) {
        // this is a mailing list message
        if (recipients.isEmpty() && !from.isEmpty()) {
            // The sender didn't set a Reply-to address, so we add the From
            // address to the list of CC recipients.
            ccRecipients += from;
            qDebug() << "Added" << from << "to the list of CC recipients";
        }

        // if it is a mailing list, add the posting address
        recipients.prepend(mailingListAddresses[0]);
    } else {
        // this is a normal message
        if (recipients.isEmpty() && !from.isEmpty()) {
            // in case of replying to a normal message only then add the From
            // address to the list of recipients if there was no Reply-to address
            recipients += from;
            qDebug() << "Added" << from << "to the list of recipients";
        }
    }

    // strip all my addresses from the list of recipients
    toList = stripMyAddressesFromAddressList(recipients, me);

    // merge To header and CC header into a list of CC recipients
    auto appendToCcRecipients = [&](const KMime::Types::Mailbox::List & list) {
        foreach (const KMime::Types::Mailbox &mailbox, list) {
            if (!recipients.contains(mailbox) && !ccRecipients.contains(mailbox)) {
                ccRecipients += mailbox;
                qDebug() << "Added" << mailbox.prettyAddress() << "to the list of CC recipients";
            }
        }
    };
    appendToCcRecipients(to);
    appendToCcRecipients(cc);

    if (!ccRecipients.isEmpty()) {
        // strip all my addresses from the list of CC recipients
        ccRecipients = stripMyAddressesFromAddressList(ccRecipients, me);

        // in case of a reply to self, toList might be empty. if that's the case
        // then propagate a cc recipient to To: (if there is any).
        if (toList.isEmpty() && !ccRecipients.isEmpty()) {
            toList << ccRecipients.at(0);
            ccRecipients.pop_front();
        }

        ccList = ccRecipients;
    }

    if (toList.isEmpty() && !recipients.isEmpty()) {
        // reply to self without other recipients
        toList << recipients.at(0);
    }

    return {toList, ccList};
}

void MailTemplates::reply(const KMime::Message::Ptr &origMsg, const std::function<void(const KMime::Message::Ptr &result)> &callback, const KMime::Types::AddrSpecList &me)
{
    //FIXME
    const bool alwaysPlain = true;

    // Decrypt what we have to
    MimeTreeParser::ObjectTreeParser otp;
    otp.parseObjectTree(origMsg.data());
    otp.decryptAndVerify();

    auto partList = otp.collectContentParts();
    if (partList.isEmpty()) {
        Q_ASSERT(false);
        return;
    }
    auto part = partList[0];
    Q_ASSERT(part);

    // Prepare the reply message
    KMime::Message::Ptr msg(new KMime::Message);

    msg->removeHeader<KMime::Headers::To>();
    msg->removeHeader<KMime::Headers::Subject>();
    msg->contentType(true)->setMimeType("text/plain");
    msg->contentType()->setCharset("utf-8");

    auto getMailboxes = [](const KMime::Headers::Base *h) -> KMime::Types::Mailbox::List {
        if (h) {
            return static_cast<const KMime::Headers::Generics::AddressList*>(h)->mailboxes();
        }
        return {};
    };

    auto fromHeader = static_cast<const KMime::Headers::From*>(part->header(KMime::Headers::From::staticType()));
    const auto recipients = getRecipients(
        fromHeader ? fromHeader->mailboxes() : KMime::Types::Mailbox::List{},
        getMailboxes(part->header(KMime::Headers::To::staticType())),
        getMailboxes(part->header(KMime::Headers::Cc::staticType())),
        getMailboxes(part->header(KMime::Headers::ReplyTo::staticType())),
        getMailingListAddresses(part->header("List-Post")),
        me
    );
    for (const auto &mailbox : recipients.to) {
        msg->to()->addAddress(mailbox);
    }
    for (const auto &mailbox : recipients.cc) {
        msg->cc(true)->addAddress(mailbox);
    }

    const auto messageId = as7BitString(part->header(KMime::Headers::MessageID::staticType()));

    const QByteArray refStr = getRefStr(as7BitString(part->header(KMime::Headers::References::staticType())), messageId);
    if (!refStr.isEmpty()) {
        msg->references()->fromUnicodeString(QString::fromLocal8Bit(refStr), "utf-8");
    }

    //In-Reply-To = original msg-id
    msg->inReplyTo()->from7BitString(messageId);


    const auto subjectHeader = part->header(KMime::Headers::Subject::staticType());
    msg->subject()->fromUnicodeString(replySubject(asUnicodeString(subjectHeader)), "utf-8");

    auto definedLocale = QLocale::system();

    //Add quoted body
    QString plainBody;
    QString htmlBody;

    //On $datetime you wrote:
    auto dateHeader = static_cast<const KMime::Headers::Date*>(part->header(KMime::Headers::Date::staticType()));
    const QDateTime date = dateHeader ? dateHeader->dateTime() : QDateTime{};
    const auto dateTimeString = QString("%1 %2").arg(definedLocale.toString(date.date(), QLocale::LongFormat)).arg(definedLocale.toString(date.time(), QLocale::LongFormat));
    const auto onDateYouWroteLine = QString("On %1 you wrote:\n").arg(dateTimeString);
    plainBody.append(onDateYouWroteLine);
    htmlBody.append(plainToHtml(onDateYouWroteLine));

    const auto plainTextContent = otp.plainTextContent();
    const auto htmlContent = otp.htmlContent();

    plainMessageText(plainTextContent, htmlContent, [=] (const QString &body) {
        QString result = stripSignature(body);
        //Quoted body
        result = quotedPlainText(result,  fromHeader ? fromHeader->displayString() : QString{});
        if (result.endsWith(QLatin1Char('\n'))) {
            result.chop(1);
        }
        //The plain body is complete
        auto plainBodyResult = plainBody + result;
        htmlMessageText(plainTextContent, htmlContent, [=] (const QString &body, const QString &headElement) {
            QString result = stripSignature(body);

            //The html body is complete
            const auto htmlBodyResult = [&]() {
                if (!alwaysPlain) {
                    auto htmlBodyResult = htmlBody + quotedHtmlText(result);
                    makeValidHtml(htmlBodyResult, headElement);
                    return htmlBodyResult;
                }
                return QString{};
            }();

            //Assemble the message
            msg->contentType()->clear(); // to get rid of old boundary

            KMime::Content *const mainTextPart =
                htmlBodyResult.isEmpty() ?
                createPlainPartContent(plainBodyResult, msg.data()) :
                createMultipartAlternativeContent(plainBodyResult, htmlBodyResult, msg.data());
            mainTextPart->assemble();

            msg->setBody(mainTextPart->encodedBody());
            msg->setHeader(mainTextPart->contentType());
            msg->setHeader(mainTextPart->contentTransferEncoding());
            //FIXME this does more harm than good right now.
            // applyCharset(msg, origMsg);
            msg->assemble();

            callback(msg);
        });
    });
}

void MailTemplates::forward(const KMime::Message::Ptr &origMsg,
    const std::function<void(const KMime::Message::Ptr &result)> &callback)
{
    MimeTreeParser::ObjectTreeParser otp;
    otp.parseObjectTree(origMsg.data());
    otp.decryptAndVerify();


    KMime::Message::Ptr wrapperMsg(new KMime::Message);
    wrapperMsg->to()->clear();
    wrapperMsg->cc()->clear();

    // Decrypt the original message, it will be encrypted again in the composer
    // for the right recipient
    KMime::Message::Ptr forwardedMessage(new KMime::Message());

    if (isEncrypted(origMsg.data())) {
        qDebug() << "Original message was encrypted, decrypting it";

        auto htmlContent = otp.htmlContent();

        KMime::Content *recreatedMsg =
            htmlContent.isEmpty() ? createPlainPartContent(otp.plainTextContent()) :
                                    createMultipartAlternativeContent(otp.plainTextContent(), htmlContent);

        KMime::Message::Ptr tmpForwardedMessage;
        auto attachments = otp.collectAttachmentParts();
        if (!attachments.isEmpty()) {
            QVector<KMime::Content *> contents = {recreatedMsg};
            for (const auto &attachment : attachments) {
                //Copy the node, to avoid deleting the parts node.
                auto c = new KMime::Content;
                c->setContent(attachment->node()->encodedContent());
                c->parse();
                contents.append(c);
            }

            auto msg = createMultipartMixedContent(contents);

            tmpForwardedMessage.reset(KMime::contentToMessage(msg));
        } else {
            tmpForwardedMessage.reset(KMime::contentToMessage(recreatedMsg));
        }

        origMsg->contentType()->fromUnicodeString(tmpForwardedMessage->contentType()->asUnicodeString(), "utf-8");
        origMsg->assemble();
        forwardedMessage->setHead(origMsg->head());
        forwardedMessage->setBody(tmpForwardedMessage->encodedBody());
        forwardedMessage->parse();
    } else {
        qDebug() << "Original message was not encrypted, using it as-is";
        forwardedMessage = origMsg;
    }

    auto partList = otp.collectContentParts();
    if (partList.isEmpty()) {
        Q_ASSERT(false);
        callback({});
        return;
    }
    auto part = partList[0];
    Q_ASSERT(part);

    const auto subjectHeader = part->header(KMime::Headers::Subject::staticType());
    const auto subject = asUnicodeString(subjectHeader);

    const QByteArray refStr = getRefStr(
        as7BitString(part->header(KMime::Headers::References::staticType())),
        as7BitString(part->header(KMime::Headers::MessageID::staticType()))
    );

    wrapperMsg->subject()->fromUnicodeString(forwardSubject(subject), "utf-8");

    if (!refStr.isEmpty()) {
        wrapperMsg->references()->fromUnicodeString(QString::fromLocal8Bit(refStr), "utf-8");
    }

    KMime::Content *fwdAttachment = new KMime::Content;

    fwdAttachment->contentDisposition()->setDisposition(KMime::Headers::CDinline);
    fwdAttachment->contentType()->setMimeType("message/rfc822");
    fwdAttachment->contentDisposition()->setFilename(subject + ".eml");
    fwdAttachment->setBody(KMime::CRLFtoLF(forwardedMessage->encodedContent(false)));

    wrapperMsg->addContent(fwdAttachment);
    wrapperMsg->assemble();

    callback(wrapperMsg);
}

QString MailTemplates::plaintextContent(const KMime::Message::Ptr &msg)
{
    MimeTreeParser::ObjectTreeParser otp;
    otp.parseObjectTree(msg.data());
    otp.decryptAndVerify();
    const auto plain = otp.plainTextContent();
    if (plain.isEmpty()) {
        //Maybe not as good as the webengine version, but works at least for simple html content
        return toPlainText(otp.htmlContent());
    }
    return plain;
}

QString MailTemplates::body(const KMime::Message::Ptr &msg, bool &isHtml)
{
    MimeTreeParser::ObjectTreeParser otp;
    otp.parseObjectTree(msg.data());
    otp.decryptAndVerify();
    const auto html = otp.htmlContent();
    if (html.isEmpty()) {
        isHtml = false;
        return otp.plainTextContent();
    }
    isHtml = true;
    return html;
}

static KMime::Content *createAttachmentPart(const QByteArray &content, const QString &filename, bool isInline, const QByteArray &mimeType, const QString &name, bool base64Encode = true)
{

    KMime::Content *part = new KMime::Content;
    part->contentDisposition(true)->setFilename(filename);
    if (isInline) {
        part->contentDisposition(true)->setDisposition(KMime::Headers::CDinline);
    } else {
        part->contentDisposition(true)->setDisposition(KMime::Headers::CDattachment);
    }

    part->contentType(true)->setMimeType(mimeType);
    if (!name.isEmpty()) {
        part->contentType(true)->setName(name, "utf-8");
    }
    if(base64Encode) {
        part->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEbase64);
    }
    part->setBody(content);
    return part;
}

static KMime::Content *createBodyPart(const QString &body, bool htmlBody) {
    if (htmlBody) {
        return createMultipartAlternativeContent(toPlainText(body), body);
    }
    return createPlainPartContent(body);
}

static KMime::Types::Mailbox::List stringListToMailboxes(const QStringList &list)
{
    KMime::Types::Mailbox::List mailboxes;
    for (const auto &s : list) {
        KMime::Types::Mailbox mb;
        mb.fromUnicodeString(s);
        if (mb.hasAddress()) {
            mailboxes << mb;
        } else {
            qWarning() << "Got an invalid address: " << s << list;
            Q_ASSERT(false);
        }
    }
    return mailboxes;
}


static void setRecipients(KMime::Message &message, const Recipients &recipients)
{
    message.to(true)->clear();
    for (const auto &mb : stringListToMailboxes(recipients.to)) {
        message.to()->addAddress(mb);
    }
    message.cc(true)->clear();
    for (const auto &mb : stringListToMailboxes(recipients.cc)) {
        message.cc()->addAddress(mb);
    }
    message.bcc(true)->clear();
    for (const auto &mb : stringListToMailboxes(recipients.bcc)) {
        message.bcc()->addAddress(mb);
    }
}


KMime::Message::Ptr MailTemplates::createMessage(KMime::Message::Ptr existingMessage,
    const QStringList &to, const QStringList &cc, const QStringList &bcc,
    const KMime::Types::Mailbox &from, const QString &subject, const QString &body, bool htmlBody,
    const QList<Attachment> &attachments, const std::vector<Crypto::Key> &signingKeys,
    const std::vector<Crypto::Key> &encryptionKeys, const Crypto::Key &attachedKey)
{
    auto mail = existingMessage;
    if (!mail) {
        mail = KMime::Message::Ptr::create();
    } else {
        //Content type is part of the body part we're creating
        mail->removeHeader<KMime::Headers::ContentType>();
        mail->removeHeader<KMime::Headers::ContentTransferEncoding>();
    }

    mail->date()->setDateTime(QDateTime::currentDateTime());
    mail->userAgent()->fromUnicodeString(QString("%1/%2(%3)").arg(QString::fromLocal8Bit("Kube")).arg("0.1").arg(QSysInfo::prettyProductName()), "utf-8");

    setRecipients(*mail, {to, cc, bcc});

    mail->from(true)->clear();
    mail->from(true)->addAddress(from);

    mail->subject(true)->fromUnicodeString(subject, "utf-8");
    if (!mail->messageID(false)) {
        //A globally unique messageId that doesn't leak the local hostname
        const auto messageId = "<" + QUuid::createUuid().toString().mid(1, 36).remove('-') + "@kube>";
        mail->messageID(true)->fromUnicodeString(messageId, "utf-8");
    }
    if (!mail->date(true)->dateTime().isValid()) {
        mail->date(true)->setDateTime(QDateTime::currentDateTimeUtc());
    }
    mail->assemble();

    const bool encryptionRequired = !signingKeys.empty() || !encryptionKeys.empty();
    //We always attach the key when encryption is enabled.
    const bool attachingPersonalKey = encryptionRequired;

    auto allAttachments = attachments;
    if (attachingPersonalKey) {
        const auto publicKeyExportResult = Crypto::exportPublicKey(attachedKey);
        if (!publicKeyExportResult) {
            qWarning() << "Failed to export public key" << publicKeyExportResult.error();
            return {};
        }
        const auto publicKeyData = publicKeyExportResult.value();
        allAttachments << Attachment{
            {},
            QString("0x%1.asc").arg(QString{attachedKey.shortKeyId}),
            "application/pgp-keys",
            false,
            publicKeyData
        };
    }

    std::unique_ptr<KMime::Content> bodyPart{[&] {
        if (!allAttachments.isEmpty()) {
            auto bodyPart = new KMime::Content;
            bodyPart->contentType(true)->setMimeType("multipart/mixed");
            bodyPart->contentType()->setBoundary(KMime::multiPartBoundary());
            bodyPart->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
            bodyPart->setPreamble("This is a multi-part message in MIME format.\n");
            bodyPart->addContent(createBodyPart(body, htmlBody));
            for (const auto &attachment : allAttachments) {

                // Just always encode attachments base64 so it's safe for binary data,
                // except when it's another message or an ascii armored key
                static QSet<QString> noEncodingRequired{{"message/rfc822"}, {"application/pgp-keys"}};
                const bool base64Encode = !noEncodingRequired.contains(attachment.mimeType);
                bodyPart->addContent(createAttachmentPart(attachment.data, attachment.filename, attachment.isInline, attachment.mimeType, attachment.name, base64Encode));
            }
            return bodyPart;
        } else {
            return createBodyPart(body, htmlBody);
        }
    }()};
    bodyPart->assemble();

    const QByteArray bodyData = [&] {
        if (encryptionRequired) {
            auto result = MailCrypto::processCrypto(std::move(bodyPart), signingKeys, encryptionKeys);
            if (!result) {
                qWarning() << "Crypto failed" << result.error();
                return QByteArray{};
            }
            result.value()->assemble();
            return result.value()->encodedContent();
        } else {
            if (!bodyPart->contentType(false)) {
                bodyPart->contentType(true)->setMimeType("text/plain");
                bodyPart->assemble();
            }
            return bodyPart->encodedContent();
        }
    }();
    if (bodyData.isEmpty()) {
        return {};
    }

    KMime::Message::Ptr resultMessage(new KMime::Message);
    resultMessage->setContent(mail->head() + bodyData);
    resultMessage->parse(); // Not strictly necessary.
    return resultMessage;
}


KMime::Message::Ptr MailTemplates::createIMipMessage(
    const QString &from,
    const Recipients &recipients,
    const QString &subject,
    const QString &body,
    const QString &attachment)
{
    KMime::Message::Ptr message = KMime::Message::Ptr( new KMime::Message );
    message->contentTransferEncoding()->clear();  // 7Bit, decoded.

    // Set the headers
    message->userAgent()->fromUnicodeString(QString("%1/%2(%3)").arg(QString::fromLocal8Bit("Kube")).arg("0.1").arg(QSysInfo::prettyProductName()), "utf-8");
    message->from()->fromUnicodeString(from, "utf-8");

    setRecipients(*message, recipients);

    message->date()->setDateTime(QDateTime::currentDateTime());
    message->subject()->fromUnicodeString(subject, "utf-8");
    message->contentType()->setMimeType("multipart/alternative");
    message->contentType()->setBoundary(KMime::multiPartBoundary());

    // Set the first multipart, the body message.
    KMime::Content *bodyMessage = new KMime::Content{message.data()};
    bodyMessage->contentType()->setMimeType("text/plain");
    bodyMessage->contentType()->setCharset("utf-8");
    bodyMessage->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
    bodyMessage->setBody(KMime::CRLFtoLF(body.toUtf8()));
    message->addContent( bodyMessage );

    // Set the second multipart, the attachment.
    KMime::Content *attachMessage = new KMime::Content{message.data()};
    attachMessage->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
    attachMessage->contentType()->setMimeType("text/calendar");
    attachMessage->contentType()->setCharset("utf-8");
    attachMessage->contentType()->setName(QLatin1String("event.ics"), "utf-8");
    attachMessage->contentType()->setParameter(QLatin1String("method"), QLatin1String("REPLY"));
    attachMessage->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
    attachMessage->setBody(KMime::CRLFtoLF(attachment.toUtf8()));
    message->addContent(attachMessage);

    // Job done, attach the both multiparts and assemble the message.
    message->assemble();
    return message;
}
