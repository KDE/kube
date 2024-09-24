/*
    Copyright (c) 2016 Sandro Knauß <knauss@kolabsys.com>

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

#include "partmodel.h"

#include <sink/mimetreeparser/objecttreeparser.h>
#include "htmlutils.h"

#include <QDebug>
#include <QTextDocument>
#include <QRegularExpression>

std::pair<QString, bool> PartModel::trim(const QString &text)
{
    //The delimiters have <p>.? prefixed including the .? because sometimes we get a byte order mark <feff> (seen with user-agent: Microsoft-MacOutlook/10.1d.0.190908)
    //We match both regulard withspace with \s and non-breaking spaces with \u00A0
    const QList<QRegularExpression> delimiters{
        QRegularExpression{"<p>.?-+Original(\\s|\u00A0)Message-+", QRegularExpression::CaseInsensitiveOption},
        QRegularExpression{"<p>.?-+Ursprüngliche(\\s|\u00A0)Nachricht-+", QRegularExpression::CaseInsensitiveOption},

        //Forwarded
        QRegularExpression{"<p>.?Von:.*</p>", QRegularExpression::CaseInsensitiveOption},
        QRegularExpression{"<p>.?From:.*</p>", QRegularExpression::CaseInsensitiveOption},
        //Reply
        //The remainder is not quoted
        QRegularExpression{"<p>.?On.*wrote:", QRegularExpression::CaseInsensitiveOption},
        //The remainder is quoted
        QRegularExpression{"&gt; On.*wrote:", QRegularExpression::CaseInsensitiveOption},
        QRegularExpression{"<p>.?Am.*schrieb.*:</p>", QRegularExpression::CaseInsensitiveOption},
        //Signature
        QRegularExpression{"<p>.?--(\\s|\u00A0)<br>", QRegularExpression::CaseInsensitiveOption}
    };
    for (const auto &expression : delimiters) {
        auto i = expression.globalMatch(text);
        while (i.hasNext()) {
            const auto match = i.next();
            const int startOffset = match.capturedStart(0);
            // This is a very simplistic detection for an inline reply where we would have the patterns before the actual message content.
            // We simply ignore anything we find within the first few lines.
            if (startOffset >= 5) {
                return {text.mid(0, startOffset), true};
            } else {
                //Search for the next delimiter
                continue;
            }
        }
    }

    return {text, false};
}

static QString addCss(const QString &s)
{
    //Get the default font from QApplication
    static const auto fontFamily = QFont{}.family();
    //overflow:hidden ensures no scrollbars are ever shown.
    static const auto css = QString("<style>\n")
               + QString("body {\n"
               "  overflow:hidden;\n"
               "  font-family: \"%1\" ! important;\n"
               "  color: #31363b ! important;\n"
               "  background-color: #fcfcfc ! important\n"
               "}\n").arg(fontFamily)
               + QString("blockquote { \n"
               "  border-left: 2px solid #bdc3c7 ! important;\n"
               "}\n")
               + QString("</style>");

    const auto header = QLatin1String("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
                  "<html><head><title></title>")
                  + css + QLatin1String("</head>\n<body>\n");
    return header + s + QStringLiteral("</body></html>");
}

class PartModelPrivate
{
public:
    PartModelPrivate(PartModel *q_ptr, const std::shared_ptr<MimeTreeParser::ObjectTreeParser> &parser)
        : q(q_ptr)
        , mParser(parser)
    {
        collectContents();
    }


    ~PartModelPrivate() = default;

    void checkPart(const MimeTreeParser::MessagePart::Ptr part) {
        //Extract the content of the part and
        mContents.insert(part.data(), extractContent(part.data()));
    }

    //Recursively find encapsulated messages
    void findEncapsulated(const MimeTreeParser::EncapsulatedRfc822MessagePart::Ptr &e) {
        mEncapsulatedParts[e.data()] = mParser->collectContentParts(e);
        for (auto subPart : mEncapsulatedParts[e.data()]) {
            checkPart(subPart);
            mParents[subPart.data()] = e.data();
            if (auto encapsulatedSub = subPart.dynamicCast<MimeTreeParser::EncapsulatedRfc822MessagePart>()) {
                findEncapsulated(encapsulatedSub);
            }
        }
    }

    QVariant extractContent(MimeTreeParser::MessagePart *messagePart)
    {
        if (auto alternativePart = dynamic_cast<MimeTreeParser::AlternativeMessagePart*>(messagePart)) {
            if (alternativePart->availableModes().contains(MimeTreeParser::AlternativeMessagePart::MultipartIcal)) {
                return alternativePart->icalContent();
            }
        }

        auto preprocessPlaintext = [&] (const QString &text) {
            //We alwas do richtext (so we get highlighted links and stuff). NOTE: this inserts non-breaking spaces instead of regular spaces.
            const auto html = Qt::convertFromPlainText(text);
            if (trimMail) {
                const auto result = PartModel::trim(html);
                isTrimmed = result.second;
                emit q->trimMailChanged();
                return HtmlUtils::linkify(result.first);
            }
            return HtmlUtils::linkify(html);
        };

        if (messagePart->isHtml()) {
            if (dynamic_cast<MimeTreeParser::AlternativeMessagePart*>(messagePart)) {
                containsHtmlAndPlain = true;
                emit q->containsHtmlChanged();
                if (!showHtml) {
                    return preprocessPlaintext(messagePart->plaintextContent());
                }
            }
            return addCss(mParser->resolveCidLinks(messagePart->htmlContent()));
        }
        return preprocessPlaintext(messagePart->text());
    }

    QVariant contentForPart(MimeTreeParser::MessagePart *messagePart) const
    {
        return mContents.value(messagePart);
    }

    void collectContents()
    {
        mEncapsulatedParts.clear();
        mParents.clear();
        mContents.clear();
        containsHtmlAndPlain = false;
        isTrimmed = false;

        mParts = mParser->collectContentParts();
        for (auto p : mParts) {
            checkPart(p);
            if (auto e = p.dynamicCast<MimeTreeParser::EncapsulatedRfc822MessagePart>()) {
                findEncapsulated(e);
            }
        }
    }

    PartModel *q;
    QVector<MimeTreeParser::MessagePartPtr> mParts;
    QHash<MimeTreeParser::MessagePart*, QVector<MimeTreeParser::MessagePartPtr>> mEncapsulatedParts;
    QHash<MimeTreeParser::MessagePart*, MimeTreeParser::MessagePart*> mParents;
    QMap<MimeTreeParser::MessagePart*, QVariant> mContents;
    std::shared_ptr<MimeTreeParser::ObjectTreeParser> mParser;
    bool showHtml{false};
    bool containsHtmlAndPlain{false};
#ifdef KUBE_EXPERIMENTAL
    bool trimMail{true};
#else
    bool trimMail{false};
#endif
    bool isTrimmed{false};
};

PartModel::PartModel(std::shared_ptr<MimeTreeParser::ObjectTreeParser> parser)
    : d(std::unique_ptr<PartModelPrivate>(new PartModelPrivate(this, parser)))
{
}

PartModel::~PartModel()
{
}

void PartModel::setShowHtml(bool html)
{
    if (d->showHtml != html) {
        beginResetModel();
        d->showHtml = html;
        d->collectContents();
        endResetModel();
        emit showHtmlChanged();
    }
}

bool PartModel::showHtml() const
{
    return d->showHtml;
}

void PartModel::setTrimMail(bool trim)
{
    if (d->trimMail != trim) {
        beginResetModel();
        d->trimMail = trim;
        d->collectContents();
        endResetModel();
        emit trimMailChanged();
    }
}

bool PartModel::trimMail() const
{
    return d->trimMail;
}

bool PartModel::isTrimmed() const
{
    return d->isTrimmed;
}

bool PartModel::containsHtml() const
{
    return d->containsHtmlAndPlain;
}

QHash<int, QByteArray> PartModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[ContentRole] = "content";
    roles[IsEmbeddedRole] = "embedded";
    roles[IsEncryptedRole] = "encrypted";
    roles[IsSignedRole] = "signed";
    roles[SecurityLevelRole] = "securityLevel";
    roles[EncryptionSecurityLevelRole] = "encryptionSecurityLevel";
    roles[SignatureSecurityLevelRole] = "signatureSecurityLevel";
    roles[ErrorType] = "errorType";
    roles[ErrorString] = "errorString";
    roles[IsErrorRole] = "error";
    roles[SenderRole] = "sender";
    roles[SignatureDetails] = "signatureDetails";
    roles[EncryptionDetails] = "encryptionDetails";
    roles[DateRole] = "date";
    return roles;
}

QModelIndex PartModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }
    if (parent.isValid()) {
        if (auto e = dynamic_cast<MimeTreeParser::EncapsulatedRfc822MessagePart*>(static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer()))) {
            const auto parts = d->mEncapsulatedParts[e];
            if (row < parts.size()) {
                return createIndex(row, column, parts.at(row).data());
            }
        }
        return QModelIndex();
    }
    if (row < d->mParts.size()) {
        return createIndex(row, column, d->mParts.at(row).data());
    }
    return QModelIndex();
}

SignatureInfo *encryptionInfo(MimeTreeParser::MessagePart *messagePart)
{
    auto signatureInfo = new SignatureInfo;
    const auto encryptions = messagePart->encryptions();
    if (encryptions.size() > 1) {
        qWarning() << "Can't deal with more than one encryption";
    }
    for (const auto &p : encryptions) {
        signatureInfo->keyId = p->partMetaData()->keyId;
    }
    return signatureInfo;
};

SignatureInfo *signatureInfo(MimeTreeParser::MessagePart *messagePart)
{
    auto signatureInfo = new SignatureInfo;
    const auto signatureParts = messagePart->signatures();
    if (signatureParts.size() > 1) {
        qWarning() << "Can't deal with more than one signature";
    }
    for (const auto &p : signatureParts) {
        signatureInfo->keyId = p->partMetaData()->keyId;
        signatureInfo->keyMissing = p->partMetaData()->keyMissing;
        signatureInfo->keyExpired = p->partMetaData()->keyExpired;
        signatureInfo->keyRevoked = p->partMetaData()->keyRevoked;
        signatureInfo->sigExpired = p->partMetaData()->sigExpired;
        signatureInfo->crlMissing = p->partMetaData()->crlMissing;
        signatureInfo->crlTooOld = p->partMetaData()->crlTooOld;
        signatureInfo->signer = p->partMetaData()->signer;
        signatureInfo->signerMailAddresses = p->partMetaData()->signerMailAddresses;
        signatureInfo->signatureIsGood = p->partMetaData()->isGoodSignature;
        signatureInfo->keyIsTrusted = p->partMetaData()->keyIsTrusted;
    }
    return signatureInfo;
}

QVariant PartModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.internalPointer()) {
        const auto messagePart = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
        // qWarning() << "Found message part " << messagePart->metaObject()->className() << messagePart->partMetaData()->status << messagePart->error();
        Q_ASSERT(messagePart);
        switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Content%1");
            case SenderRole: {
                if (auto e = dynamic_cast<MimeTreeParser::EncapsulatedRfc822MessagePart*>(messagePart)) {
                    return e->from();
                }
                return {};
            }
            case DateRole: {
                if (auto e = dynamic_cast<MimeTreeParser::EncapsulatedRfc822MessagePart*>(messagePart)) {
                    return e->date();
                }
                return {};
            }
            case TypeRole: {
                if (messagePart->error()) {
                    return "error";
                }
                if (dynamic_cast<MimeTreeParser::EncapsulatedRfc822MessagePart*>(messagePart)) {
                    return "encapsulated";
                }
                if (auto alternativePart = dynamic_cast<MimeTreeParser::AlternativeMessagePart*>(messagePart)) {
                    if (alternativePart->availableModes().contains(MimeTreeParser::AlternativeMessagePart::MultipartIcal)) {
                        return "ical";
                    }
                }
                if (!d->showHtml && d->containsHtmlAndPlain) {
                    return "plain";
                }
                //For simple html we don't need a browser
                auto complexHtml = [&] {
                    if (messagePart->isHtml()) {
                        const auto text = messagePart->htmlContent();
                        if (text.contains("<!DOCTYPE html PUBLIC")) {
                            //We can probably deal with this if it adheres to the strict dtd
                            //(that's what our composer produces as well)
                            if (!text.contains("http://www.w3.org/TR/REC-html40/strict.dtd")) {
                                return true;
                            }
                        }
                        //Blockquotes don't support any styling which would be necessary so they become readable.
                        if (text.contains("blockquote")) {
                            return true;
                        }
                        //Media queries are too advanced
                        if (text.contains("@media")) {
                            return true;
                        }
                        //auto css properties are not supported e.g margin-left: auto;
                        if (text.contains(": auto;")) {
                            return true;
                        }
                        return false;
                    } else {
                        return false;
                    }
                }();
                if (complexHtml) {
                    return "html";
                }
                return "plain";
            }
            case IsEmbeddedRole:
                return false;
            case IsErrorRole:
                return messagePart->error();
            case ContentRole:
                return d->contentForPart(messagePart);
            case IsEncryptedRole:
                return messagePart->encryptionState() != MimeTreeParser::KMMsgNotEncrypted;
            case IsSignedRole:
                return messagePart->signatureState() != MimeTreeParser::KMMsgNotSigned;
            case SecurityLevelRole: {
                auto signature = messagePart->signatureState();
                auto encryption = messagePart->encryptionState();
                bool messageIsSigned = signature == MimeTreeParser::KMMsgPartiallySigned ||
                                       signature == MimeTreeParser::KMMsgFullySigned;
                bool messageIsEncrypted = encryption == MimeTreeParser::KMMsgPartiallyEncrypted ||
                                          encryption == MimeTreeParser::KMMsgFullyEncrypted;

                if (messageIsSigned) {
                    auto sigInfo = std::unique_ptr<SignatureInfo>{signatureInfo(messagePart)};
                    if (!sigInfo->signatureIsGood) {
                        if (sigInfo->keyMissing || sigInfo->keyExpired) {
                            return "notsogood";
                        }
                        return "bad";
                    }
                }
                //All good
                if (messageIsSigned || messageIsEncrypted) {
                    return "good";
                }
                //No info
                return "unknown";
            }
            case EncryptionSecurityLevelRole: {
                auto encryption = messagePart->encryptionState();
                bool messageIsEncrypted = encryption == MimeTreeParser::KMMsgPartiallyEncrypted ||
                                          encryption == MimeTreeParser::KMMsgFullyEncrypted;
                if (messagePart->error()) {
                    return "bad";
                }
                //All good
                if (messageIsEncrypted) {
                    return "good";
                }
                //No info
                return "unknown";
            }
            case SignatureSecurityLevelRole: {
                auto signature = messagePart->signatureState();
                bool messageIsSigned = signature == MimeTreeParser::KMMsgPartiallySigned ||
                                       signature == MimeTreeParser::KMMsgFullySigned;
                if (messageIsSigned) {
                    auto sigInfo = std::unique_ptr<SignatureInfo>{signatureInfo(messagePart)};
                    if (!sigInfo->signatureIsGood) {
                        if (sigInfo->keyMissing || sigInfo->keyExpired) {
                            return "notsogood";
                        }
                        return "bad";
                    }
                    return "good";
                }
                //No info
                return "unknown";

            }
            case SignatureDetails:
                return QVariant::fromValue(signatureInfo(messagePart));
            case EncryptionDetails:
                return QVariant::fromValue(encryptionInfo(messagePart));
            case ErrorType:
                return messagePart->error();
            case ErrorString: {
                switch (messagePart->error()) {
                    case MimeTreeParser::MessagePart::NoKeyError:
                        return tr("No key available.");
                    case MimeTreeParser::MessagePart::PassphraseError:
                        return tr("Wrong passphrase.");
                    case MimeTreeParser::MessagePart::UnknownError:
                        break;
                    default:
                        break;
                }
                return messagePart->errorString();
            }
        }
    }
    return QVariant();
}

QModelIndex PartModel::parent(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (auto e = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer())) {
            for (const auto &p : d->mParts) {
                if (p.data() == e) {
                    return QModelIndex();
                }
            }
            const auto parentPart = d->mParents[e];
            Q_ASSERT(parentPart);
            int row = 0;
            const auto parts = d->mEncapsulatedParts[parentPart];
            for (const auto &p : parts) {
                if (p.data() == e) {
                    break;
                }
                row++;
            }
            return createIndex(row, 0, parentPart);
        }
        return {};
    }
    return {};
}

int PartModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (auto e = dynamic_cast<MimeTreeParser::EncapsulatedRfc822MessagePart*>(static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer()))) {
            const auto parts = d->mEncapsulatedParts[e];
            return parts.size();
        }
        return 0;
    }
    return d->mParts.count();
}

int PartModel::columnCount(const QModelIndex &) const
{
    return 1;
}
