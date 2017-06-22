/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include <QTextCodec>
#include <QTextDocument>

#include <KCodecs/KCharsets>
#include <KMime/Types>

#include <mimetreeparser/objecttreeparser.h>

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

void initHeader(const KMime::Message::Ptr &message)
{
    message->removeHeader<KMime::Headers::To>();
    message->removeHeader<KMime::Headers::Subject>();
    message->date()->setDateTime(QDateTime::currentDateTime());

    const QStringList extraInfo = QStringList() << QSysInfo::prettyProductName();
    message->userAgent()->fromUnicodeString(QString("%1/%2(%3)").arg(QString::fromLocal8Bit("Kube")).arg("0.1").arg(extraInfo.join(",")), "utf-8");
    // This will allow to change Content-Type:
    message->contentType()->setMimeType("text/plain");
}

QString replacePrefixes(const QString &str, const QStringList &prefixRegExps,
                        bool replace, const QString &newPrefix)
{
    bool recognized = false;
    // construct a big regexp that
    // 1. is anchored to the beginning of str (sans whitespace)
    // 2. matches at least one of the part regexps in prefixRegExps
    QString bigRegExp = QStringLiteral("^(?:\\s+|(?:%1))+\\s*").arg(prefixRegExps.join(QStringLiteral(")|(?:")));
    QRegExp rx(bigRegExp, Qt::CaseInsensitive);
    if (rx.isValid()) {
        QString tmp = str;
        if (rx.indexIn(tmp) == 0) {
            recognized = true;
            if (replace) {
                return tmp.replace(0, rx.matchedLength(), newPrefix + QLatin1String(" "));
            }
        }
    } else {
        qWarning() << "bigRegExp = \""
                    << bigRegExp << "\"\n"
                    << "prefix regexp is invalid!";
        // try good ole Re/Fwd:
        recognized = str.startsWith(newPrefix);
    }

    if (!recognized) {
        return newPrefix + QLatin1String(" ") + str;
    } else {
        return str;
    }
}

QString cleanSubject(const KMime::Message::Ptr &msg, const QStringList &prefixRegExps, bool replace, const QString &newPrefix)
{
    return replacePrefixes(msg->subject()->asUnicodeString(), prefixRegExps, replace, newPrefix);
}

QString forwardSubject(const KMime::Message::Ptr &msg)
{
    bool replaceForwardPrefix = true;
    QStringList forwardPrefixes;
    forwardPrefixes << "Fwd:";
    forwardPrefixes << "FW:";
    return cleanSubject(msg, forwardPrefixes, replaceForwardPrefix, QStringLiteral("Fwd:"));
}

QString replySubject(const KMime::Message::Ptr &msg)
{
    bool replaceReplyPrefix = true;
    QStringList replyPrefixes;
    //We're escaping the regex escape sequences. awesome
    replyPrefixes << "Re\\\\s*:";
    replyPrefixes << "Re[\\\\d+\\\\]:";
    replyPrefixes << "Re\\\\d+:";
    return cleanSubject(msg, replyPrefixes, replaceReplyPrefix, QStringLiteral("Re:"));
}

QByteArray getRefStr(const KMime::Message::Ptr &msg)
{
    QByteArray firstRef, lastRef, refStr, retRefStr;
    int i, j;

    if (auto hdr = msg->references(false)) {
        refStr = hdr->as7BitString(false).trimmed();
    }

    if (refStr.isEmpty()) {
        return msg->messageID()->as7BitString(false);
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

    retRefStr += msg->messageID()->as7BitString(false);
    return retRefStr;
}

KMime::Content *createPlainPartContent(const KMime::Message::Ptr &msg, const QString &plainBody)
{
    KMime::Content *textPart = new KMime::Content(msg.data());
    textPart->contentType()->setMimeType("text/plain");
    //FIXME This is supposed to select a charset out of the available charsets that contains all necessary characters to render the text
    // QTextCodec *charset = selectCharset(m_charsets, plainBody);
    // textPart->contentType()->setCharset(charset->name());
    textPart->contentType()->setCharset("utf-8");
    textPart->contentTransferEncoding()->setEncoding(KMime::Headers::CE8Bit);
    textPart->fromUnicodeString(plainBody);
    return textPart;
}

KMime::Content *createMultipartAlternativeContent(const KMime::Message::Ptr &msg, const QString &plainBody, const QString &htmlBody)
{
    KMime::Content *multipartAlternative = new KMime::Content(msg.data());
    multipartAlternative->contentType()->setMimeType("multipart/alternative");
    const QByteArray boundary = KMime::multiPartBoundary();
    multipartAlternative->contentType()->setBoundary(boundary);

    KMime::Content *textPart = createPlainPartContent(msg, plainBody);
    multipartAlternative->addContent(textPart);

    KMime::Content *htmlPart = new KMime::Content(msg.data());
    htmlPart->contentType()->setMimeType("text/html");
    //FIXME This is supposed to select a charset out of the available charsets that contains all necessary characters to render the text
    // QTextCodec *charset = selectCharset(m_charsets, htmlBody);
    // htmlPart->contentType()->setCharset(charset->name());
    textPart->contentType()->setCharset("utf-8");
    htmlPart->contentTransferEncoding()->setEncoding(KMime::Headers::CE8Bit);
    htmlPart->fromUnicodeString(htmlBody);
    multipartAlternative->addContent(htmlPart);

    return multipartAlternative;
}

void addProcessedBodyToMessage(const KMime::Message::Ptr &msg, const QString &plainBody, const QString &htmlBody, bool forward)
{
    //FIXME
    // MessageCore::ImageCollector ic;
    // ic.collectImagesFrom(mOrigMsg.data());

    // Now, delete the old content and set the new content, which
    // is either only the new text or the new text with some attachments.
    auto parts = msg->contents();
    foreach (KMime::Content *content, parts) {
        msg->removeContent(content, true/*delete*/);
    }

    msg->contentType()->clear(); // to get rid of old boundary

    const QByteArray boundary = KMime::multiPartBoundary();
    KMime::Content *const mainTextPart =
        htmlBody.isEmpty() ?
        createPlainPartContent(msg, plainBody) :
        createMultipartAlternativeContent(msg, plainBody, htmlBody);
    mainTextPart->assemble();

    KMime::Content *textPart = mainTextPart;
    // if (!ic.images().empty()) {
    //     textPart = createMultipartRelated(ic, mainTextPart);
    //     textPart->assemble();
    // }

    // If we have some attachments, create a multipart/mixed mail and
    // add the normal body as well as the attachments
    KMime::Content *mainPart = textPart;
    //FIXME
    // if (forward) {
    //     auto attachments = mOrigMsg->attachments();
    //     attachments += mOtp->nodeHelper()->attachmentsOfExtraContents();
    //     if (!attachments.isEmpty()) {
    //         mainPart = createMultipartMixed(attachments, textPart);
    //         mainPart->assemble();
    //     }
    // }

    msg->setBody(mainPart->encodedBody());
    msg->setHeader(mainPart->contentType());
    msg->setHeader(mainPart->contentTransferEncoding());
    msg->assemble();
    msg->parse();
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
QString stripSignature(const QString &msg)
{
    // Following RFC 3676, only > before --
    // I prefer to not delete a SB instead of delete good mail content.
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

void setupPage(QWebEnginePage *page)
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
        page->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
        page->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
        page->settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
#endif
}

void plainMessageText(const QString &plainTextContent, const QString &htmlContent, bool aStripSignature, const std::function<void(const QString &)> &callback)
{
    QString result = plainTextContent;
    if (plainTextContent.isEmpty()) {   //HTML-only mails
        auto page = new QWebEnginePage;
        setupPage(page);
        page->setHtml(htmlContent);
        page->toPlainText([=] (const QString &plaintext) {
            page->deleteLater();
            callback(plaintext);
        });
        return;
    }

    if (aStripSignature) {
        result = stripSignature(result);
    }
    callback(result);
}

QString extractHeaderBodyScript()
{
    const QString source = QStringLiteral("(function() {"
                                          "var res = {"
                                          "    body: document.getElementsByTagName('body')[0].innerHTML,"
                                          "    header: document.getElementsByTagName('head')[0].innerHTML"
                                          "};"
                                          "return res;"
                                          "})()");
    return source;
}

void htmlMessageText(const QString &plainTextContent, const QString &htmlContent, bool aStripSignature, const std::function<void(const QString &body, QString &head)> &callback)
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
            if (aStripSignature) {
                callback(stripSignature(bodyElement), headerElement);
            }
            return callback(bodyElement, headerElement);
        }

        if (aStripSignature) {
            return callback(stripSignature(htmlElement), headerElement);
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

void MailTemplates::reply(const KMime::Message::Ptr &origMsg, const std::function<void(const KMime::Message::Ptr &result)> &callback)
{
    //FIXME
    const bool alwaysPlain = true;
    //FIXME
    const ReplyStrategy replyStrategy = ReplySmart;
    KMime::Message::Ptr msg(new KMime::Message);
    //FIXME
    //Personal email addresses
    KMime::Types::AddrSpecList me;
    KMime::Types::Mailbox::List toList;
    KMime::Types::Mailbox::List replyToList;
    KMime::Types::Mailbox::List mailingListAddresses;

    // const uint originalIdentity = identityUoid(origMsg);
    initHeader(msg);
    replyToList = origMsg->replyTo()->mailboxes();

    msg->contentType()->setCharset("utf-8");

    if (origMsg->headerByType("List-Post") &&
            origMsg->headerByType("List-Post")->asUnicodeString().contains(QStringLiteral("mailto:"), Qt::CaseInsensitive)) {

        const QString listPost = origMsg->headerByType("List-Post")->asUnicodeString();
        QRegExp rx(QStringLiteral("<mailto:([^@>]+)@([^>]+)>"), Qt::CaseInsensitive);
        if (rx.indexIn(listPost, 0) != -1) {   // matched
            KMime::Types::Mailbox mailbox;
            mailbox.fromUnicodeString(rx.cap(1) + QLatin1Char('@') + rx.cap(2));
            mailingListAddresses << mailbox;
        }
    }

    switch (replyStrategy) {
    case ReplySmart: {
        if (auto hdr = origMsg->headerByType("Mail-Followup-To")) {
            toList << KMime::Types::Mailbox::listFrom7BitString(hdr->as7BitString(false));
        } else if (!replyToList.isEmpty()) {
            toList = replyToList;
        } else if (!mailingListAddresses.isEmpty()) {
            toList = (KMime::Types::Mailbox::List() << mailingListAddresses.at(0));
        } else {
            // doesn't seem to be a mailing list, reply to From: address
            toList = origMsg->from()->mailboxes();

            bool listContainsMe = false;
            for (const auto &m : me) {
                KMime::Types::Mailbox mailbox;
                mailbox.setAddress(m);
                if (toList.contains(mailbox)) {
                    listContainsMe = true;
                }
            }
            if (listContainsMe) {
                // sender seems to be one of our own identities, so we assume that this
                // is a reply to a "sent" mail where the users wants to add additional
                // information for the recipient.
                toList = origMsg->to()->mailboxes();
            }
        }
        // strip all my addresses from the list of recipients
        const KMime::Types::Mailbox::List recipients = toList;

        toList = stripMyAddressesFromAddressList(recipients, me);

        // ... unless the list contains only my addresses (reply to self)
        if (toList.isEmpty() && !recipients.isEmpty()) {
            toList << recipients.first();
        }
    }
    break;
    case ReplyList: {
        if (auto hdr = origMsg->headerByType("Mail-Followup-To")) {
            KMime::Types::Mailbox mailbox;
            mailbox.from7BitString(hdr->as7BitString(false));
            toList << mailbox;
        } else if (!mailingListAddresses.isEmpty()) {
            toList << mailingListAddresses[ 0 ];
        } else if (!replyToList.isEmpty()) {
            // assume a Reply-To header mangling mailing list
            toList = replyToList;
        }

        //FIXME
        // strip all my addresses from the list of recipients
        const KMime::Types::Mailbox::List recipients = toList;
        toList = stripMyAddressesFromAddressList(recipients, me);
    }
    break;
    case ReplyAll: {
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
            if (recipients.isEmpty() && !origMsg->from()->asUnicodeString().isEmpty()) {
                // The sender didn't set a Reply-to address, so we add the From
                // address to the list of CC recipients.
                ccRecipients += origMsg->from()->mailboxes();
                qDebug() << "Added" << origMsg->from()->asUnicodeString() << "to the list of CC recipients";
            }

            // if it is a mailing list, add the posting address
            recipients.prepend(mailingListAddresses[ 0 ]);
        } else {
            // this is a normal message
            if (recipients.isEmpty() && !origMsg->from()->asUnicodeString().isEmpty()) {
                // in case of replying to a normal message only then add the From
                // address to the list of recipients if there was no Reply-to address
                recipients += origMsg->from()->mailboxes();
                qDebug() << "Added" << origMsg->from()->asUnicodeString() << "to the list of recipients";
            }
        }

        // strip all my addresses from the list of recipients
        toList = stripMyAddressesFromAddressList(recipients, me);

        // merge To header and CC header into a list of CC recipients
        if (!origMsg->cc()->asUnicodeString().isEmpty() || !origMsg->to()->asUnicodeString().isEmpty()) {
            KMime::Types::Mailbox::List list;
            if (!origMsg->to()->asUnicodeString().isEmpty()) {
                list += origMsg->to()->mailboxes();
            }
            if (!origMsg->cc()->asUnicodeString().isEmpty()) {
                list += origMsg->cc()->mailboxes();
            }

            foreach (const KMime::Types::Mailbox &mailbox, list) {
                if (!recipients.contains(mailbox) &&
                        !ccRecipients.contains(mailbox)) {
                    ccRecipients += mailbox;
                    qDebug() << "Added" << mailbox.prettyAddress() << "to the list of CC recipients";
                }
            }
        }

        if (!ccRecipients.isEmpty()) {
            // strip all my addresses from the list of CC recipients
            ccRecipients = stripMyAddressesFromAddressList(ccRecipients, me);

            // in case of a reply to self, toList might be empty. if that's the case
            // then propagate a cc recipient to To: (if there is any).
            if (toList.isEmpty() && !ccRecipients.isEmpty()) {
                toList << ccRecipients.at(0);
                ccRecipients.pop_front();
            }

            foreach (const KMime::Types::Mailbox &mailbox, ccRecipients) {
                msg->cc()->addAddress(mailbox);
            }
        }

        if (toList.isEmpty() && !recipients.isEmpty()) {
            // reply to self without other recipients
            toList << recipients.at(0);
        }
    }
    break;
    case ReplyAuthor: {
        if (!replyToList.isEmpty()) {
            KMime::Types::Mailbox::List recipients = replyToList;

            // strip the mailing list post address from the list of Reply-To
            // addresses since we want to reply in private
            foreach (const KMime::Types::Mailbox &mailbox, mailingListAddresses) {
                foreach (const KMime::Types::Mailbox &recipient, recipients) {
                    if (mailbox == recipient) {
                        recipients.removeAll(recipient);
                    }
                }
            }

            if (!recipients.isEmpty()) {
                toList = recipients;
            } else {
                // there was only the mailing list post address in the Reply-To header,
                // so use the From address instead
                toList = origMsg->from()->mailboxes();
            }
        } else if (!origMsg->from()->asUnicodeString().isEmpty()) {
            toList = origMsg->from()->mailboxes();
        }
    }
    break;
    case ReplyNone:
        // the addressees will be set by the caller
        break;
    }

    foreach (const KMime::Types::Mailbox &mailbox, toList) {
        msg->to()->addAddress(mailbox);
    }

    const QByteArray refStr = getRefStr(origMsg);
    if (!refStr.isEmpty()) {
        msg->references()->fromUnicodeString(QString::fromLocal8Bit(refStr), "utf-8");
    }

    //In-Reply-To = original msg-id
    msg->inReplyTo()->from7BitString(origMsg->messageID()->as7BitString(false));

    msg->subject()->fromUnicodeString(replySubject(origMsg), "utf-8");

    auto definedLocale = QLocale::system();

    //Add quoted body
    QString plainBody;
    QString htmlBody;

    //On $datetime you wrote:
    const QDateTime date = origMsg->date()->dateTime();
    const auto dateTimeString = QString("%1 %2").arg(definedLocale.toString(date.date(), QLocale::LongFormat)).arg(definedLocale.toString(date.time(), QLocale::LongFormat));
    const auto onDateYouWroteLine = QString("On %1 you wrote:").arg(dateTimeString);
    plainBody.append(onDateYouWroteLine);
    htmlBody.append(plainToHtml(onDateYouWroteLine));

    //Strip signature for replies
    const bool stripSignature = true;

    MimeTreeParser::ObjectTreeParser otp;
    otp.parseObjectTree(origMsg.data());
    const auto plainTextContent = otp.plainTextContent();
    const auto htmlContent = otp.htmlContent();

    plainMessageText(plainTextContent, htmlContent, stripSignature, [=] (const QString &body) {
        //Quoted body */
        QString plainQuote = quotedPlainText(body, origMsg->from()->displayString());
        if (plainQuote.endsWith(QLatin1Char('\n'))) {
            plainQuote.chop(1);
        }
        //The plain body is complete */
        auto plainBodyResult = plainBody + plainQuote;
        htmlMessageText(plainTextContent, htmlContent, stripSignature, [=] (const QString &body, const QString &headElement) {
            //The html body is complete
            auto htmlBodyResult = htmlBody + quotedHtmlText(body);
            if (alwaysPlain) {
                htmlBodyResult.clear();
            } else {
                makeValidHtml(htmlBodyResult, headElement);
            }

            //Assemble the message */
            addProcessedBodyToMessage(msg, plainBodyResult, htmlBodyResult, false);
            applyCharset(msg, origMsg);
            msg->assemble();
            //We're done
            callback(msg);
        });
    });
}

QString MailTemplates::plaintextContent(const KMime::Message::Ptr &msg)
{
    MimeTreeParser::ObjectTreeParser otp;
    otp.parseObjectTree(msg.data());
    const auto plain = otp.plainTextContent();
    if (plain.isEmpty()) {
        //Maybe not as good as the webengine version, but works at least for simple html content
        QTextDocument doc;
        doc.setHtml(otp.htmlContent());
        return doc.toPlainText();
    }
    return plain;
}
