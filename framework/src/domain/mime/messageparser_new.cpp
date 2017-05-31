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

#include "messageparser.h"
#include "mimetreeparser/interface.h"
#include "mimetreeparser/otp/objecttreeparser.h"
#include "mimetreeparser/otp/messagepart.h"
#include "htmlutils.h"

#include <QDebug>
#include <QMimeDatabase>
#include <QTextDocument>

class NewModelPrivate
{
public:
    NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<Parser> &parser);
    ~NewModelPrivate();

    void createTree();
    NewModel *q;
    QVector<MimeTreeParser::Interface::MessagePartPtr> mParts;
    std::shared_ptr<Parser> mParser;
};

NewModelPrivate::NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<Parser> &parser)
    : q(q_ptr)
    , mParser(parser)
{
    mParts = mParser->otp()->collectContentParts();
    qWarning() << "Collected content parts: " << mParts.size();
}

NewModelPrivate::~NewModelPrivate()
{
}

NewModel::NewModel(std::shared_ptr<Parser> parser)
    : d(std::unique_ptr<NewModelPrivate>(new NewModelPrivate(this, parser)))
{
}

NewModel::~NewModel()
{
}

QHash<int, QByteArray> NewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[ContentRole] = "content";
    roles[IsComplexHtmlContentRole] = "complexHtmlContent";
    roles[IsEmbededRole] = "embeded";
    roles[SecurityLevelRole] = "securityLevel";
    roles[EncryptionErrorType] = "errorType";
    roles[EncryptionErrorString] = "errorString";
    return roles;
}

QModelIndex NewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }
    if (row < d->mParts.size()) {
        return createIndex(row, column, d->mParts.at(row).data());
    }
    return QModelIndex();
}

QVariant NewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        switch (role) {
        case Qt::DisplayRole:
            return QString("root");
        case IsEmbededRole:
            return false;
        }
        return QVariant();
    }

    if (index.internalPointer()) {
        const auto messagePart = dynamic_cast<MimeTreeParser::MessagePart*>(static_cast<MimeTreeParser::Interface::MessagePart*>(index.internalPointer()));
        Q_ASSERT(messagePart);
        // if (_data->userType() ==  qMetaTypeId<Signature *>()) {
        //     const auto signature = _data->value<Signature *>();
        //     // int i = d->getPos(signature);
        //     switch(role) {
        //     case Qt::DisplayRole:
        //         return QStringLiteral("Signature%1");
        //     case TypeRole:
        //         return QStringLiteral("Signature");
        //     case SecurityLevelRole:
        //         return QStringLiteral("RED");
        //     case IsEmbededRole:
        //         return data(index.parent(), IsEmbededRole);
        //     }
        // } else if (_data->userType() ==  qMetaTypeId<Encryption *>()) {
        //     const auto encryption = _data->value<Encryption *>();
        //     switch(role) {
        //     case Qt::DisplayRole:
        //         return QStringLiteral("Encryption%1");
        //     case TypeRole:
        //         return QStringLiteral("Encryption");
        //     case SecurityLevelRole:
        //         return QStringLiteral("GREEN");
        //     case IsEmbededRole:
        //         return data(index.parent(), IsEmbededRole);
        //     case EncryptionErrorType:
        //         {
        //             switch(encryption->errorType()) {
        //             case Encryption::NoError:
        //                 return QString();
        //             case Encryption::PassphraseError:
        //                 return QStringLiteral("PassphraseError");
        //             case Encryption::KeyMissing:
        //                 return QStringLiteral("KeyMissing");
        //             default:
        //                 return QStringLiteral("UnknownError");
        //             }
        //         }
        //     case EncryptionErrorString:
        //         return encryption->errorString();
        //     }
        // if (_data->userType() ==  qMetaTypeId<Part *>()) {
        //     const auto part = _data->value<Part *>();
        //     switch (role) {
        //     case Qt::DisplayRole:
        //     case TypeRole:
        //         return QString::fromLatin1(part->type());
        //     case IsEmbededRole:
        //         return data(index.parent(), IsEmbededRole);
        //     }
        
        // const auto content = _data->value<Content *>();
        switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Content%1");
            case TypeRole:
                // TODO this is matched in the maildatamodel
                return "Content";
            case IsEmbededRole:
                return false;
            case IsComplexHtmlContentRole: {
                if (messagePart->isHtml()) {
                    const auto text = messagePart->text();
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
                break;
            }
            case ContentRole: {
                auto text = messagePart->isHtml() ? messagePart->htmlContent() : messagePart->text();
                qWarning() << "Encoded content: " << text;
                if (messagePart->isHtml()) {
                    const auto rx = QRegExp("(src)\\s*=\\s*(\"|')(cid:[^\"']+)\\2");
                    int pos = 0;
                    while ((pos = rx.indexIn(text, pos)) != -1) {
                        qWarning() << "Found a match at : " << pos;
                        const auto link = QUrl(rx.cap(3).toUtf8());
                        pos += rx.matchedLength();
                        //FIXME this fails for inline forwarded messages
                        auto cid = link.path();
                        auto mailMime = const_cast<KMime::Content *>(d->mParser->otp()->find([=] (KMime::Content *c) {
                            if (!c || !c->contentID(false)) {
                                return false;
                            }
                            qWarning() << "Looking at: " << c->contentID(false)->identifier();
                            return c->contentID(false)->identifier() == cid;
                        }));


                        // const auto repl = d->mParser->getPart(link);
                        // if (!repl) {
                        //     qWarning() << "no part for link : " << link;
                        //     continue;
                        // }
                        // const auto content = repl->content();
                        // if(content.size() < 1) {
                        //     qWarning() << "too small : " << content.size();
                        //     continue;
                        // }
                        // const auto mailMime = content.first()->mailMime();
                        if (mailMime) {
                            // const auto mimetype = mailMime->mimetype().name();

                            const auto ct = mailMime->contentType(false);
                            if (ct) {
                                continue;
                            }

                            QMimeDatabase mimeDb;
                            auto mimetype = mimeDb.mimeTypeForName(ct->mimeType()).name();

                            if (mimetype.startsWith("image/")) {
                                const auto data = mailMime->encodedContent();
                                text.replace(rx.cap(0), QString("src=\"data:%1;base64,%2\"").arg(mimetype, QString::fromLatin1(data.toBase64())));
                            }
                        }
                    }
                } else { //We assume plain
                    //We alwas do richtext (so we get highlighted links and stuff).
                    return HtmlUtils::linkify(Qt::convertFromPlainText(text));
                }
                return text;
            }
        }
    }
    return QVariant();
}

QModelIndex NewModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int NewModel::rowCount(const QModelIndex &parent) const
{
    return d->mParts.count();
}

int NewModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}
