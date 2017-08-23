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

#include <mimetreeparser/objecttreeparser.h>
#include "htmlutils.h"

#include <QDebug>
#include <QMimeDatabase>
#include <QTextDocument>

class PartModelPrivate
{
public:
    PartModelPrivate(PartModel *q_ptr, const std::shared_ptr<MimeTreeParser::ObjectTreeParser> &parser);
    ~PartModelPrivate();

    void findEncapsulated(const MimeTreeParser::EncapsulatedRfc822MessagePart::Ptr &e);
    PartModel *q;
    QVector<MimeTreeParser::MessagePartPtr> mParts;
    QHash<MimeTreeParser::MessagePart*, QVector<MimeTreeParser::MessagePartPtr>> mEncapsulatedParts;
    QHash<MimeTreeParser::MessagePart*, MimeTreeParser::MessagePart*> mParents;
    std::shared_ptr<MimeTreeParser::ObjectTreeParser> mParser;
};

PartModelPrivate::PartModelPrivate(PartModel *q_ptr, const std::shared_ptr<MimeTreeParser::ObjectTreeParser> &parser)
    : q(q_ptr)
    , mParser(parser)
{
    mParts = mParser->collectContentParts();
    for (auto p : mParts) {
        if (auto e = p.dynamicCast<MimeTreeParser::EncapsulatedRfc822MessagePart>()) {
            findEncapsulated(e);
        }
    }
}

//Recursively find encapsulated messages
void PartModelPrivate::findEncapsulated(const MimeTreeParser::EncapsulatedRfc822MessagePart::Ptr &e)
{
    mEncapsulatedParts[e.data()] = mParser->collectContentParts(e);
    for (auto subPart : mEncapsulatedParts[e.data()]) {
        mParents[subPart.data()] = e.data();
        if (auto encapsulatedSub = subPart.dynamicCast<MimeTreeParser::EncapsulatedRfc822MessagePart>()) {
            findEncapsulated(encapsulatedSub);
        }
    }
};

PartModelPrivate::~PartModelPrivate()
{
}

PartModel::PartModel(std::shared_ptr<MimeTreeParser::ObjectTreeParser> parser)
    : d(std::unique_ptr<PartModelPrivate>(new PartModelPrivate(this, parser)))
{
}

PartModel::~PartModel()
{
}

QHash<int, QByteArray> PartModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[ContentRole] = "content";
    roles[IsEmbededRole] = "embeded";
    roles[IsEncryptedRole] = "encrypted";
    roles[IsSignedRole] = "signed";
    roles[SecurityLevelRole] = "securityLevel";
    roles[EncryptionErrorType] = "errorType";
    roles[EncryptionErrorString] = "errorString";
    roles[IsErrorRole] = "error";
    roles[SenderRole] = "sender";
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

static QString addCss(const QString &s)
{
    //Get the default font from QApplication
    static const auto fontFamily = QFont{}.family();
    //overflow:hidden ensures no scrollbars are ever shown.
    static const auto css = QString("<style>\n"
               "body {\n"
               "  overflow:hidden;\n"
               "  font-family: \"%1\" ! important;\n"
               "  color: #31363b ! important;\n"
               "  background-color: #fcfcfc ! important\n"
               "}\n"
               "</style>")
        .arg(fontFamily);

    const auto header = QLatin1String("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
                  "<html><head><title></title>")
                  + css + QLatin1String("</head>\n<body>\n");
    return header + s + QStringLiteral("</body></html>");
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
                //For simple html we don't need a browser
                auto complexHtml = [&] {
                    if (messagePart->isHtml()) {
                        const auto text = messagePart->htmlContent();
                        if (text.contains("<!DOCTYPE html PUBLIC")) {
                            return true;
                        }
                        //Media queries are too advanced
                        if (text.contains("@media")) {
                            return true;
                        }
                        if (text.contains("<style")) {
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
            case IsEmbededRole:
                return false;
            case IsErrorRole:
                return messagePart->error();
            case ContentRole: {
                const auto text = messagePart->isHtml() ? messagePart->htmlContent() : messagePart->text();
                if (messagePart->isHtml()) {
                    return addCss(d->mParser->resolveCidLinks(text));
                } else { //We assume plain
                    //We alwas do richtext (so we get highlighted links and stuff).
                    return HtmlUtils::linkify(Qt::convertFromPlainText(text));
                }
                return text;
            }
            case IsEncryptedRole:
                return messagePart->encryptionState() != MimeTreeParser::KMMsgNotEncrypted;
            case IsSignedRole:
                return messagePart->signatureState() != MimeTreeParser::KMMsgNotSigned;
            case EncryptionErrorType:
                return messagePart->error();
            case EncryptionErrorString: {
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

int PartModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}
