/* Copyright 2009 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stringutil.h"

#include "config-enterprise.h"
#include "MessageCore/MessageCoreSettings"

#include <kmime/kmime_header_parsing.h>
#include <kmime/kmime_headers.h>
#include <kmime/kmime_message.h>
#include <KEmailAddress>

#include <KConfigGroup>
#include "messagecore_debug.h"
#include <KUser>

#include <QHostInfo>
#include <QRegExp>
#include <QStringList>
#include <QUrlQuery>
#include <kpimtextedit/textutils.h>

using namespace KMime;
using namespace KMime::Types;
using namespace KMime::HeaderParsing;

namespace MessageCore
{

namespace StringUtil
{

// Removes trailing spaces and tabs at the end of the line
static void removeTrailingSpace(QString &line)
{
    int i = line.length() - 1;
    while ((i >= 0) && ((line[i] == QLatin1Char(' ')) || (line[i] == QLatin1Char('\t')))) {
        i--;
    }
    line.truncate(i + 1);
}

// Splits the line off in two parts: The quote prefixes and the actual text of the line.
// For example, for the string "> > > Hello", it would be split up in "> > > " as the quote
// prefix, and "Hello" as the actual text.
// The actual text is written back to the "line" parameter, and the quote prefix is returned.
static QString splitLine(QString &line)
{
    removeTrailingSpace(line);
    int i = 0;
    int startOfActualText = -1;

    // TODO: Replace tabs with spaces first.

    // Loop through the chars in the line to find the place where the quote prefix stops
    const int lineLength(line.length());
    while (i < lineLength) {
        const QChar c = line[i];
        const bool isAllowedQuoteChar = (c == QLatin1Char('>')) || (c == QLatin1Char(':')) || (c == QLatin1Char('|')) ||
                                        (c == QLatin1Char(' ')) || (c == QLatin1Char('\t'));
        if (isAllowedQuoteChar) {
            startOfActualText = i + 1;
        } else {
            break;
        }
        ++i;
    }

    // If the quote prefix only consists of whitespace, don't consider it as a quote prefix at all
    if (line.left(startOfActualText).trimmed().isEmpty()) {
        startOfActualText = 0;
    }

    // No quote prefix there -> nothing to do
    if (startOfActualText <= 0) {
        return QString();
    }

    // Entire line consists of only the quote prefix
    if (i == line.length()) {
        const QString quotePrefix = line.left(startOfActualText);
        line.clear();
        return quotePrefix;
    }

    // Line contains both the quote prefix and the actual text, really split it up now
    const QString quotePrefix = line.left(startOfActualText);
    line = line.mid(startOfActualText);

    return quotePrefix;
}

// Writes all lines/text parts contained in the "textParts" list to the output text, "msg".
// Quote characters are added in front of each line, and no line is longer than
// maxLength.
//
// Although the lines in textParts are considered separate lines, they can actually be run
// together into a single line in some cases. This is basically the main difference to flowText().
//
// Example:
//   textParts = "Hello World, this is a test.", "Really"
//   indent = ">"
//   maxLength = 20
//   Result: "> Hello World, this\n
//            > is a test. Really"
// Notice how in this example, the text line "Really" is no longer a separate line, it was run
// together with a previously broken line.
//
// "textParts" is cleared upon return.
static bool flushPart(QString &msg, QStringList &textParts,
                      const QString &indent, int maxLength)
{
    if (maxLength < 20) {
        maxLength = 20;
    }

    // Remove empty lines at end of quote
    while (!textParts.isEmpty() && textParts.last().isEmpty()) {
        textParts.removeLast();
    }

    QString text;
    foreach (const QString &line, textParts) {

        // An empty line in the input means that an empty line should be in the output as well.
        // Therefore, we write all of our text so far to the msg.
        if (line.isEmpty()) {
            if (!text.isEmpty()) {
                msg += KPIMTextEdit::TextUtils::flowText(text, indent, maxLength) + QLatin1Char('\n');
            }
            msg += indent + QLatin1Char('\n');
        } else {
            if (text.isEmpty()) {
                text = line;
            } else {
                text += QLatin1Char(' ') + line.trimmed();
            }
            // If the line doesn't need to be wrapped at all, just write it out as-is.
            // When a line exceeds the maximum length and therefore needs to be broken, this statement
            // if false, and therefore we keep adding lines to our text, so they get ran together in the
            // next flowText call, as "text" contains several text parts/lines then.
            if ((text.length() < maxLength) || (line.length() < (maxLength - 10))) {
                msg += KPIMTextEdit::TextUtils::flowText(text, indent, maxLength) + QLatin1Char('\n');
            }
        }
    }

    // Write out pending text to the msg
    if (!text.isEmpty()) {
        msg += KPIMTextEdit::TextUtils::flowText(text, indent, maxLength);
    }

    const bool appendEmptyLine = !textParts.isEmpty();
    textParts.clear();

    return appendEmptyLine;
}

QMap<QString, QString> parseMailtoUrl(const QUrl &url)
{
    QMap<QString, QString> values;
    if (url.scheme() != QLatin1String("mailto")) {
        return values;
    }
    QUrlQuery query(url);
    Q_FOREACH (const auto &queryItem, query.queryItems()) {
        values.insert(queryItem.first, queryItem.second);
    }

    QStringList to;
    to << KEmailAddress::decodeMailtoUrl(url);

    const QString toStr = values.value(QStringLiteral("to"));
    if (!toStr.isEmpty()) {
        to << toStr;
    }

    values.insert(QStringLiteral("to"), to.join(QStringLiteral(", ")));
    return values;
}

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

AddressList splitAddressField(const QByteArray &text)
{
    AddressList result;
    const char *begin = text.begin();
    if (!begin) {
        return result;
    }

    const char *const end = text.begin() + text.length();

    if (!parseAddressList(begin, end, result)) {
        qCDebug(MESSAGECORE_LOG) << "Error in address splitting: parseAddressList returned false!";
    }

    return result;
}

QString generateMessageId(const QString &address, const QString &suffix)
{
    const QDateTime dateTime = QDateTime::currentDateTime();

    QString msgIdStr = QLatin1Char('<') + dateTime.toString(QStringLiteral("yyyyMMddhhmm.sszzz"));

    if (!suffix.isEmpty()) {
        msgIdStr += QLatin1Char('@') + suffix;
    } else {
        msgIdStr += QLatin1Char('.') + KEmailAddress::toIdn(address);
    }

    msgIdStr += QLatin1Char('>');

    return msgIdStr;
}

QString stripEmailAddr(const QString &aStr)
{
    //qCDebug(MESSAGECORE_LOG) << "(" << aStr << ")";

    if (aStr.isEmpty()) {
        return QString();
    }

    QString result;

    // The following is a primitive parser for a mailbox-list (cf. RFC 2822).
    // The purpose is to extract a displayable string from the mailboxes.
    // Comments in the addr-spec are not handled. No error checking is done.

    QString name;
    QString comment;
    QString angleAddress;
    enum { TopLevel, InComment, InAngleAddress } context = TopLevel;
    bool inQuotedString = false;
    int commentLevel = 0;

    QChar ch;
    int strLength(aStr.length());
    for (int index = 0; index < strLength; ++index) {
        ch = aStr[index];
        switch (context) {
        case TopLevel : {
            switch (ch.toLatin1()) {
            case '"' : inQuotedString = !inQuotedString;
                break;
            case '(' : if (!inQuotedString) {
                    context = InComment;
                    commentLevel = 1;
                } else {
                    name += ch;
                }
                break;
            case '<' : if (!inQuotedString) {
                    context = InAngleAddress;
                } else {
                    name += ch;
                }
                break;
            case '\\' : // quoted character
                ++index; // skip the '\'
                if (index < aStr.length()) {
                    name += aStr[index];
                }
                break;
            case ',' : if (!inQuotedString) {
                    // next email address
                    if (!result.isEmpty()) {
                        result += QLatin1String(", ");
                    }
                    name = name.trimmed();
                    comment = comment.trimmed();
                    angleAddress = angleAddress.trimmed();
                    if (angleAddress.isEmpty() && !comment.isEmpty()) {
                        // handle Outlook-style addresses like
                        // john.doe@invalid (John Doe)
                        result += comment;
                    } else if (!name.isEmpty()) {
                        result += name;
                    } else if (!comment.isEmpty()) {
                        result += comment;
                    } else if (!angleAddress.isEmpty()) {
                        result += angleAddress;
                    }
                    name.clear();
                    comment.clear();
                    angleAddress.clear();
                } else {
                    name += ch;
                }
                break;
            default :  name += ch;
            }
            break;
        }
        case InComment : {
            switch (ch.toLatin1()) {
            case '(' : ++commentLevel;
                comment += ch;
                break;
            case ')' : --commentLevel;
                if (commentLevel == 0) {
                    context = TopLevel;
                    comment += QLatin1Char(' '); // separate the text of several comments
                } else {
                    comment += ch;
                }
                break;
            case '\\' : // quoted character
                ++index; // skip the '\'
                if (index < aStr.length()) {
                    comment += aStr[index];
                }
                break;
            default :  comment += ch;
            }
            break;
        }
        case InAngleAddress : {
            switch (ch.toLatin1()) {
            case '"' : inQuotedString = !inQuotedString;
                angleAddress += ch;
                break;
            case '>' : if (!inQuotedString) {
                    context = TopLevel;
                } else {
                    angleAddress += ch;
                }
                break;
            case '\\' : // quoted character
                ++index; // skip the '\'
                if (index < aStr.length()) {
                    angleAddress += aStr[index];
                }
                break;
            default :  angleAddress += ch;
            }
            break;
        }
        } // switch ( context )
    }
    if (!result.isEmpty()) {
        result += QLatin1String(", ");
    }
    name = name.trimmed();
    comment = comment.trimmed();
    angleAddress = angleAddress.trimmed();
    if (angleAddress.isEmpty() && !comment.isEmpty()) {
        // handle Outlook-style addresses like
        // john.doe@invalid (John Doe)
        result += comment;
    } else if (!name.isEmpty()) {
        result += name;
    } else if (!comment.isEmpty()) {
        result += comment;
    } else if (!angleAddress.isEmpty()) {
        result += angleAddress;
    }

    //qCDebug(MESSAGECORE_LOG) << "Returns \"" << result << "\"";
    return result;
}

QString quoteHtmlChars(const QString &str, bool removeLineBreaks)
{
    QString result;

    unsigned int strLength(str.length());
    result.reserve(6 * strLength); // maximal possible length
    for (unsigned int i = 0; i < strLength; ++i) {
        switch (str[i].toLatin1()) {
        case '<':
            result += QLatin1String("&lt;");
            break;
        case '>':
            result += QLatin1String("&gt;");
            break;
        case '&':
            result += QLatin1String("&amp;");
            break;
        case '"':
            result += QLatin1String("&quot;");
            break;
        case '\n':
            if (!removeLineBreaks) {
                result += QLatin1String("<br>");
            }
            break;
        case '\r':
            // ignore CR
            break;
        default:
            result += str[i];
        }
    }

    result.squeeze();
    return result;
}

void removePrivateHeaderFields(const KMime::Message::Ptr &message, bool cleanUpHeader)
{
    message->removeHeader("Status");
    message->removeHeader("X-Status");
    message->removeHeader("X-KMail-EncryptionState");
    message->removeHeader("X-KMail-SignatureState");
    message->removeHeader("X-KMail-Transport");
    message->removeHeader("X-KMail-Fcc");
    message->removeHeader("X-KMail-Redirect-From");
    message->removeHeader("X-KMail-Link-Message");
    message->removeHeader("X-KMail-Link-Type");
    message->removeHeader("X-KMail-QuotePrefix");
    message->removeHeader("X-KMail-CursorPos");
    message->removeHeader("X-KMail-Templates");
    message->removeHeader("X-KMail-Drafts");
    message->removeHeader("X-KMail-UnExpanded-To");
    message->removeHeader("X-KMail-UnExpanded-CC");
    message->removeHeader("X-KMail-UnExpanded-BCC");
    message->removeHeader("X-KMail-FccDisabled");

    if (cleanUpHeader) {
        message->removeHeader("X-KMail-Identity");
        message->removeHeader("X-KMail-Dictionary");
    }
}

QByteArray asSendableString(const KMime::Message::Ptr &originalMessage)
{
    KMime::Message::Ptr message(new KMime::Message);
    message->setContent(originalMessage->encodedContent());

    removePrivateHeaderFields(message);
    message->removeHeader<KMime::Headers::Bcc>();

    return message->encodedContent();
}

QByteArray headerAsSendableString(const KMime::Message::Ptr &originalMessage)
{
    KMime::Message::Ptr message(new KMime::Message);
    message->setContent(originalMessage->encodedContent());

    removePrivateHeaderFields(message);
    message->removeHeader<KMime::Headers::Bcc>();

    return message->head();
}

QString emailAddrAsAnchor(const KMime::Types::Mailbox::List &mailboxList,
                          Display display, const QString &cssStyle,
                          Link link, AddressMode expandable, const QString &fieldName,
                          int collapseNumber)
{
    QString result;
    int numberAddresses = 0;
    bool expandableInserted = false;

    foreach (const KMime::Types::Mailbox &mailbox, mailboxList) {
        if (!mailbox.prettyAddress().isEmpty()) {
            numberAddresses++;
            if (expandable == ExpandableAddresses && !expandableInserted && numberAddresses > collapseNumber) {
                result = QLatin1String("<span id=\"icon") + fieldName + QLatin1String("\"></span>") + result;
                result += QLatin1String("<span id=\"dots") + fieldName + QLatin1String("\">...</span><span id=\"hidden") + fieldName + QLatin1String("\">");
                expandableInserted = true;
            }

            if (link == ShowLink) {
                result += QLatin1String("<a href=\"mailto:")
                          + QString::fromLatin1(QUrl::toPercentEncoding(KEmailAddress::encodeMailtoUrl(mailbox.prettyAddress(KMime::Types::Mailbox::QuoteWhenNecessary)).path()))
                          + QLatin1String("\" ") + cssStyle + QLatin1Char('>');
            }
            if (display == DisplayNameOnly) {
                if (!mailbox.name().isEmpty()) { // Fallback to the email address when the name is not set.
                    result += quoteHtmlChars(mailbox.name(), true);
                } else {
                    result += quoteHtmlChars(mailbox.prettyAddress(), true);
                }
            } else {
                result += quoteHtmlChars(mailbox.prettyAddress(KMime::Types::Mailbox::QuoteWhenNecessary), true);
            }
            if (link == ShowLink) {
                result += QLatin1String("</a>, ");
            }
        }
    }

    // cut of the trailing ", "
    if (link == ShowLink) {
        result.truncate(result.length() - 2);
    }

    if (expandableInserted) {
        result += QLatin1String("</span>");
    }
    return result;
}

QString emailAddrAsAnchor(KMime::Headers::Generics::MailboxList *mailboxList,
                          Display display, const QString &cssStyle,
                          Link link, AddressMode expandable, const QString &fieldName,
                          int collapseNumber)
{
    Q_ASSERT(mailboxList);
    return emailAddrAsAnchor(mailboxList->mailboxes(), display, cssStyle, link, expandable, fieldName, collapseNumber);
}

QString emailAddrAsAnchor(KMime::Headers::Generics::AddressList *addressList,
                          Display display, const QString &cssStyle,
                          Link link, AddressMode expandable, const QString &fieldName,
                          int collapseNumber)
{
    Q_ASSERT(addressList);
    return emailAddrAsAnchor(addressList->mailboxes(), display, cssStyle, link, expandable, fieldName, collapseNumber);
}

bool addressIsInAddressList(const QString &address,
                            const QStringList &addresses)
{
    const QString addrSpec = KEmailAddress::extractEmailAddress(address);

    QStringList::ConstIterator end(addresses.constEnd());
    for (QStringList::ConstIterator it = addresses.constBegin(); it != end; ++it) {
        if (qstricmp(addrSpec.toUtf8().data(),
                     KEmailAddress::extractEmailAddress(*it).toUtf8().data()) == 0) {
            return true;
        }
    }

    return false;
}

QString guessEmailAddressFromLoginName(const QString &loginName)
{
    if (loginName.isEmpty()) {
        return QString();
    }

    QString address = loginName;
    address += QLatin1Char('@');
    address += QHostInfo::localHostName();

    // try to determine the real name
    const KUser user(loginName);
    if (user.isValid()) {
        const QString fullName = user.property(KUser::FullName).toString();
        address = KEmailAddress::quoteNameIfNecessary(fullName) + QLatin1String(" <") + address + QLatin1Char('>');
    }

    return address;
}

QString smartQuote(const QString &msg, int maxLineLength)
{
    // The algorithm here is as follows:
    // We split up the incoming msg into lines, and then iterate over each line.
    // We keep adding lines with the same indent ( = quote prefix, e.g. "> " ) to a
    // "textParts" list. So the textParts list contains only lines with the same quote
    // prefix.
    //
    // When all lines with the same indent are collected in "textParts", we write those out
    // to the result by calling flushPart(), which does all the nice formatting for us.

    QStringList textParts;
    QString oldIndent;
    bool firstPart = true;
    QString result;
    foreach (QString line, msg.split(QLatin1Char('\n'))) {

        // Split off the indent from the line
        const QString indent = splitLine(line);

        if (line.isEmpty()) {
            if (!firstPart) {
                textParts.append(QString());
            }
            continue;
        }

        if (firstPart) {
            oldIndent = indent;
            firstPart = false;
        }

        // The indent changed, that means we have to write everything contained in textParts to the
        // result, which we do by calling flushPart().
        if (oldIndent != indent) {

            // Check if the last non-blank line is a "From" line. A from line is the line containing the
            // attribution to a quote, e.g. "Yesterday, you wrote:". We'll just check for the last colon
            // here, to simply things.
            // If there is a From line, remove it from the textParts to that flushPart won't break it.
            // We'll manually add it to the result afterwards.
            QString fromLine;
            if (!textParts.isEmpty()) {
                for (int i = textParts.count() - 1; i >= 0; i--) {

                    // Check if we have found the From line
                    if (textParts[i].endsWith(QLatin1Char(':'))) {
                        fromLine = oldIndent + textParts[i] + QLatin1Char('\n');
                        textParts.removeAt(i);
                        break;
                    }

                    // Abort on first non-empty line
                    if (!textParts[i].trimmed().isEmpty()) {
                        break;
                    }
                }
            }

            // Write out all lines with the same indent using flushPart(). The textParts list
            // is cleared for us.
            if (flushPart(result, textParts, oldIndent, maxLineLength)) {
                if (oldIndent.length() > indent.length()) {
                    result += indent + QLatin1Char('\n');
                } else {
                    result += oldIndent + QLatin1Char('\n');
                }
            }

            if (!fromLine.isEmpty()) {
                result += fromLine;
            }

            oldIndent = indent;
        }

        textParts.append(line);
    }

    // Write out anything still pending
    flushPart(result, textParts, oldIndent, maxLineLength);

    // Remove superfluous newline which was appended in flowText
    if (!result.isEmpty() && result.endsWith(QLatin1Char('\n'))) {
        result.chop(1);
    }

    return result;
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

QString cleanFileName(const QString &name)
{
    QString fileName = name.trimmed();

    // We need to replace colons with underscores since those cause problems with
    // KFileDialog (bug in KFileDialog though) and also on Windows filesystems.
    // We also look at the special case of ": ", since converting that to "_ "
    // would look strange, simply "_" looks better.
    // https://issues.kolab.org/issue3805
    fileName.replace(QLatin1String(": "), QStringLiteral("_"));
    // replace all ':' with '_' because ':' isn't allowed on FAT volumes
    fileName.replace(QLatin1Char(':'), QLatin1Char('_'));
    // better not use a dir-delimiter in a filename
    fileName.replace(QLatin1Char('/'), QLatin1Char('_'));
    fileName.replace(QLatin1Char('\\'), QLatin1Char('_'));

#ifdef KDEPIM_ENTERPRISE_BUILD
    // replace all '.' with '_', not just at the start of the filename
    // but don't replace the last '.' before the file extension.
    int i = fileName.lastIndexOf(QLatin1Char('.'));
    if (i != -1) {
        i = fileName.lastIndexOf(QLatin1Char('.'), i - 1);
    }

    while (i != -1) {
        fileName.replace(i, 1, QLatin1Char('_'));
        i = fileName.lastIndexOf(QLatin1Char('.'), i - 1);
    }
#endif

    // replace all '~' with '_', not just leading '~' either.
    fileName.replace(QLatin1Char('~'), QLatin1Char('_'));

    return fileName;
}

QString stripOffPrefixes(const QString &subject)
{
    static QStringList defaultReplyPrefixes = QStringList() << QStringLiteral("Re\\s*:")
            << QStringLiteral("Re\\[\\d+\\]:")
            << QStringLiteral("Re\\d+:");

    static QStringList defaultForwardPrefixes = QStringList() << QStringLiteral("Fwd:")
            << QStringLiteral("FW:");

    QStringList replyPrefixes = MessageCoreSettings::self()->replyPrefixes();
    if (replyPrefixes.isEmpty()) {
        replyPrefixes = defaultReplyPrefixes;
    }

    QStringList forwardPrefixes = MessageCoreSettings::self()->forwardPrefixes();
    if (forwardPrefixes.isEmpty()) {
        forwardPrefixes = defaultForwardPrefixes;
    }

    const QStringList prefixRegExps = replyPrefixes + forwardPrefixes;

    // construct a big regexp that
    // 1. is anchored to the beginning of str (sans whitespace)
    // 2. matches at least one of the part regexps in prefixRegExps
    const QString bigRegExp = QStringLiteral("^(?:\\s+|(?:%1))+\\s*").arg(prefixRegExps.join(QStringLiteral(")|(?:")));

    static QString regExpPattern;
    static QRegExp regExp;

    regExp.setCaseSensitivity(Qt::CaseInsensitive);
    if (regExpPattern != bigRegExp) {
        // the prefixes have changed, so update the regexp
        regExpPattern = bigRegExp;
        regExp.setPattern(regExpPattern);
    }

    if (regExp.isValid()) {
        QString tmp = subject;
        if (regExp.indexIn(tmp) == 0) {
            return tmp.remove(0, regExp.matchedLength());
        }
    } else {
        qCWarning(MESSAGECORE_LOG) << "bigRegExp = \""
                                   << bigRegExp << "\"\n"
                                   << "prefix regexp is invalid!";
    }

    return subject;
}

void setEncodingFile(QUrl &url, const QString &encoding)
{
    url.addQueryItem(QStringLiteral("charset"), encoding);
}

}

}

