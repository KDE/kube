
/*
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

#include <QDebug>

Q_DECLARE_METATYPE(Part *)
Q_DECLARE_METATYPE(Content *)
Q_DECLARE_METATYPE(Signature *)
Q_DECLARE_METATYPE(Encryption *)

class NewModelPrivate
{
public:
    NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<Parser> &parser);
    
    QSharedPointer<QVariant> getVar(const std::shared_ptr<Signature> &sig);
    QSharedPointer<QVariant> getVar(const std::shared_ptr<Encryption> &enc);
    QSharedPointer<QVariant> getVar(const std::shared_ptr<Part> &part);

    int getPos(Signature *sig);
    int getPos(Encryption *enc);
    int getPos(Part *part);

    NewModel *q;
    QVector<Part::Ptr> mParts;

    std::shared_ptr<Parser> mParser;
    QMap<Part *, std::shared_ptr<NewContentModel>> mContentMap;
private:
    QMap<std::shared_ptr<Signature>, QSharedPointer<QVariant>> mSignatureMap;
    QMap<std::shared_ptr<Encryption>, QSharedPointer<QVariant>> mEncryptionMap;
    QMap<std::shared_ptr<Part>, QSharedPointer<QVariant>> mPartMap;
};

NewModelPrivate::NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<Parser> &parser)
    : q(q_ptr)
    , mParser(parser)
{
   mParts = mParser->collectContentParts();
   foreach(const auto &part, mParts) {
       mContentMap.insert(part.get(), std::shared_ptr<NewContentModel>(new NewContentModel(part, mParser)));
   }
}

QSharedPointer<QVariant> NewModelPrivate::getVar(const std::shared_ptr<Signature> &sig)
{
    if (!mSignatureMap.contains(sig)) {
        auto var = new QVariant();
        var->setValue(sig.get());
        mSignatureMap.insert(sig, QSharedPointer<QVariant>(var));
    }
    return mSignatureMap.value(sig);
}

QSharedPointer<QVariant> NewModelPrivate::getVar(const std::shared_ptr<Encryption> &enc)
{
    if (!mEncryptionMap.contains(enc)) {
        auto var = new QVariant();
        var->setValue(enc.get());
        mEncryptionMap.insert(enc, QSharedPointer<QVariant>(var));
    }
    return mEncryptionMap.value(enc);
}

QSharedPointer<QVariant> NewModelPrivate::getVar(const std::shared_ptr<Part> &part)
{
    if (!mPartMap.contains(part)) {
        auto var = new QVariant();
        var->setValue(part.get());
        mPartMap.insert(part, QSharedPointer<QVariant>(var));
    }
    return mPartMap.value(part);
}

int NewModelPrivate::getPos(Signature *signature)
{
    const auto first = mParts.first();
    int i = 0;
    foreach(const auto &sig, first->signatures()) {
        if (sig.get() == signature) {
            break;
        }
        i++;
    }
    return i;
}

int NewModelPrivate::getPos(Encryption *encryption)
{
    const auto first = mParts.first();
    int i = 0;
    foreach(const auto &enc, first->encryptions()) {
        if (enc.get() == encryption) {
            break;
        }
        i++;
    }
    return i;
}


int NewModelPrivate::getPos(Part *part)
{
    int i = 0;
    foreach(const auto &p, mParts) {
        if (p.get() == part) {
            break;
        }
        i++;
    }
    return i;
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
    roles[ContentsRole] = "contents";
    roles[IsEmbededRole] = "embeded";
    roles[SecurityLevelRole] = "securityLevel";
    return roles;
}


QModelIndex NewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }
    if (!parent.isValid()) {
        const auto first = d->mParts.first();
        if (first->signatures().size() > 0) {
            if (row == 0) {
                const auto sig = first->signatures().at(row);
                return createIndex(row, column, d->getVar(sig).data());
            }
        } else if (first->encryptions().size() > 0) {
            if (row == 0) {
                const auto enc = first->encryptions().at(row);
                return createIndex(row, column, d->getVar(enc).data());
            }
        } else {
            if (row < d->mParts.size()) {
                auto part = d->mParts.at(row);
                return createIndex(row, column, d->getVar(part).data());
            }
        }
    } else {
        if (!parent.internalPointer()) {
            return QModelIndex();
        }
        const auto data = static_cast<QVariant *>(parent.internalPointer());
        const auto first = d->mParts.first();
        int encpos = -1;
        int partpos = -1;
        if (data->userType() == qMetaTypeId<Signature *>()) {
            const auto signature = data->value<Signature *>();
            int i = d->getPos(signature);

            if (i+1 < first->signatures().size()) {
                if (row != 0) {
                    return QModelIndex();
                }
                const auto sig = first->signatures().at(i+1);
                return createIndex(0,0, d->getVar(sig).data());
            }

            if (first->encryptions().size() > 0) {
                encpos = 0;
            }
        } else if (data->userType() == qMetaTypeId<Encryption *>()) {
            const auto encryption = data->value<Encryption *>();
            encpos = d->getPos(encryption) + 1;
        }

        if (encpos > -1 && encpos < first->encryptions().size()) {
            if (row != 0) {
                return QModelIndex();
            }
            const auto enc = first->encryptions().at(encpos);
            return createIndex(0,0, d->getVar(enc).data());
        }

        if (row < d->mParts.size()) {
            auto part = d->mParts.at(row);
            auto var = new QVariant();
            var->setValue(part.get());
            return createIndex(row, column, d->getVar(part).data());
        }
    }
    return QModelIndex();
}

QVariant NewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        if (role == Qt::DisplayRole) {
            return QString("root");
        }
        return QVariant();
    }
    if (index.internalPointer()) {
        const auto data = static_cast<QVariant *>(index.internalPointer());
        if (data->userType() ==  qMetaTypeId<Signature *>()) {
            const auto signature = data->value<Signature *>();
            int i = d->getPos(signature);
            switch(role) {
            case Qt::DisplayRole:
                case TypeRole:
                return QStringLiteral("Signature%1").arg(i);
            }
        } else if (data->userType() ==  qMetaTypeId<Encryption *>()) {
            const auto first = d->mParts.first();
            const auto encryption = data->value<Encryption *>();
            int i = d->getPos(encryption);
            switch(role) {
            case Qt::DisplayRole:
                case TypeRole:
                return QStringLiteral("Encryption%1").arg(i);
            }
        } else if (data->userType() ==  qMetaTypeId<Part *>()) {
            const auto part = data->value<Part *>();
            switch (role) {
                case Qt::DisplayRole:
                case TypeRole:
                    return QString::fromLatin1(part->type());
                case IsEmbededRole:
                    return index.parent().isValid();
                case SecurityLevelRole:
                    return QStringLiteral("GRAY");
                case ContentsRole:
                    return  QVariant::fromValue<QAbstractItemModel *>(d->mContentMap.value(part).get());
            }
        }
    }
    return QVariant();
}

QModelIndex NewModel::parent(const QModelIndex &index) const
{
    if (!index.internalPointer()) {
        return QModelIndex();
    }
    const auto data = static_cast<QVariant *>(index.internalPointer());
    if (data->userType() == qMetaTypeId<Signature *>()) {
        const auto signature = data->value<Signature *>();
        const auto first = d->mParts.first();
        int i = d->getPos(signature);

        if (i > 1) {
            const auto sig = first->signatures().at(i-1);
            return createIndex(0, 0, d->getVar(sig).data());
        }

        return QModelIndex();
    } else if (data->userType() == qMetaTypeId<Encryption *>()) {
        const auto encryption = data->value<Encryption *>();
        const auto first = d->mParts.first();
        int i = d->getPos(encryption);

        if (i > 1) {
            const auto enc = first->encryptions().at(i-1);
            return createIndex(0, 0, d->getVar(enc).data());
        }

        if (first->signatures().size() > 0) {
            const int row = first->signatures().size() - 1;
            const auto sig = first->signatures().at(row);
            return createIndex(0, 0, d->getVar(sig).data());
        } else {
            return QModelIndex();
        }
    } else if (data->userType() == qMetaTypeId<Part *>()) {
        const auto first = d->mParts.first();
        if (first->encryptions().size() > 0) {
            const int row = first->encryptions().size() - 1;
            const auto enc = first->encryptions().at(row);
            auto var = new QVariant();
            var->setValue(enc.get());
            return createIndex(0, 0, d->getVar(enc).data());
        }
        if (first->signatures().size() > 0) {
            const int row = first->signatures().size() - 1;
            const auto sig = first->signatures().at(row);
            return createIndex(0, 0, d->getVar(sig).data());
        }
        return QModelIndex();
    }
    return QModelIndex();
}

int NewModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        const auto first = d->mParts.first();
        if (first->signatures().size() > 0) {
            return 1;
        } else if (first->encryptions().size() > 0) {
            return 1;
        } else {
            return d->mParts.size();
        }
    } else {
        if (!parent.internalPointer()) {
            return 0;
        }
        const auto data = static_cast<QVariant *>(parent.internalPointer());
        if (data->userType() == qMetaTypeId<Signature *>()) {
            const auto signature = data->value<Signature *>();
            const auto first = d->mParts.first();
            int i = d->getPos(signature);

            if (i+1 < first->signatures().size()) {
                return 1;
            }

            if (first->encryptions().size() > 0) {
                return 1;
            }

            return d->mParts.size();
        } else if (data->userType() == qMetaTypeId<Encryption *>()) {
            const auto encryption = data->value<Encryption *>();
            const auto first = d->mParts.first();
            int i = d->getPos(encryption);

            if (i+1 < first->encryptions().size()) {
                return 1;
            }

            return d->mParts.size();
        }
    }
    return 0;
}

int NewModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

NewContentModel::NewContentModel(const Part::Ptr &part, const std::shared_ptr<Parser> &parser)
    : mPart(part)
    , mParser(parser)
{
}

QHash<int, QByteArray> NewContentModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[ContentRole] = "content";
    roles[IsEmbededRole] = "embeded";
    roles[SecurityLevelRole] = "securityLevel";
    return roles;
}

QModelIndex NewContentModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row < mPart->content().size()) {
            auto part = mPart->content().at(row);
            return createIndex(row, column, part.get());
        }
    }
    return QModelIndex();
}

QVariant NewContentModel::data(const QModelIndex &index, int role) const
{
    auto content = static_cast<Content *>(index.internalPointer());
    switch (role) {
        case TypeRole:
            return QString::fromLatin1(content->type());
        case IsEmbededRole:
            return false;
        case ContentRole: {
            auto text = content->encodedContent();
            if (data(index, TypeRole).toString() == "HtmlContent") {
                const auto rx = QRegExp("(src)\\s*=\\s*(\"|')(cid:[^\"']+)\\2");
                int pos = 0;
                while ((pos = rx.indexIn(text, pos)) != -1) {
                    const auto link = QUrl(rx.cap(3).toUtf8());
                    pos += rx.matchedLength();
                    const auto repl = mParser->getPart(link);
                    if (!repl) {
                        continue;
                    }
                    const auto content = repl->content();
                    if(content.size() < 1) {
                        continue;
                    }
                    const auto mailMime = content.first()->mailMime();
                    const auto mimetype = mailMime->mimetype().name();
                    if (mimetype.startsWith("image/")) {
                        const auto data = content.first()->content();
                        text.replace(rx.cap(0), QString("src=\"data:%1;base64,%2\"").arg(mimetype, QString::fromLatin1(data.toBase64())));
                    }
                }
            }
            return text;
        }
        case SecurityLevelRole:
            return content->encryptions().size() > mPart->encryptions().size() ? "red": "black"; //test for gpg inline
    }
    return QVariant();
}

QModelIndex NewContentModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int NewContentModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mPart->content().size();
    }
    return 0;
}

int NewContentModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}
