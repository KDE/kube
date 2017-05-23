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
#include "htmlutils.h"

#include <QDebug>
#include <QTextDocument>

Q_DECLARE_METATYPE(Part *)
Q_DECLARE_METATYPE(Content *)
Q_DECLARE_METATYPE(Signature *)
Q_DECLARE_METATYPE(Encryption *)

class Entry;

class NewModelPrivate
{
public:
    NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<Parser> &parser);
    ~NewModelPrivate();

    void createTree();

    QSharedPointer<QVariant> getVar(const std::shared_ptr<Signature> &sig);
    QSharedPointer<QVariant> getVar(const std::shared_ptr<Encryption> &enc);
    QSharedPointer<QVariant> getVar(const std::shared_ptr<Part> &part);
    QSharedPointer<QVariant> getVar(Part *part);
    QSharedPointer<QVariant> getVar(const std::shared_ptr<Content> &content);
    QSharedPointer<QVariant> getVar(Content *content);

    int getPos(Signature *sig);
    int getPos(Encryption *enc);
    int getPos(Part *part);
    int getPos(Content *content);

    NewModel *q;
    QVector<Part::Ptr> mParts;
    std::unique_ptr<Entry> mRoot;

    std::shared_ptr<Parser> mParser;
private:
    QMap<std::shared_ptr<Signature>, QSharedPointer<QVariant>> mSignatureMap;
    QMap<std::shared_ptr<Encryption>, QSharedPointer<QVariant>> mEncryptionMap;
    QMap<Part *, QSharedPointer<QVariant>> mPartMap;
    QMap<Content *, QSharedPointer<QVariant>> mCMap;
};

class Entry
{
public:
    Entry(NewModelPrivate *model)
        : mParent(nullptr)
        , mNewModelPrivate(model)
    {
    }

    ~Entry()
    {
        foreach(auto child, mChildren) {
            delete child;
        }
        mChildren.clear();
    }

    void addChild(Entry *entry)
    {
        mChildren.append(entry);
        entry->mParent = this;
    }

    Entry *addSignatures(QVector<Signature::Ptr> signatures)
    {
        auto ret = this;
        foreach(const auto &sig, signatures) {
            auto entry = new Entry(mNewModelPrivate);
            entry->mData = mNewModelPrivate->getVar(sig);
            ret->addChild(entry);
            ret = entry;
        }
        return ret;
    }

    Entry *addEncryptions(QVector<Encryption::Ptr> encryptions)
    {
        auto ret = this;
        foreach(const auto &enc, encryptions) {
            auto entry = new Entry(mNewModelPrivate);
            entry->mData = mNewModelPrivate->getVar(enc);
            ret->addChild(entry);
            ret = entry;
        }
        return ret;
    }

     Entry *addPart(Part *part)
    {
        auto entry = new Entry(mNewModelPrivate);
        entry->mData = mNewModelPrivate->getVar(part);
        addChild(entry);

        foreach(const auto &content, part->content()) {
            auto _entry = entry;
            _entry = _entry->addEncryptions(content->encryptions().mid(part->encryptions().size()));
            _entry = _entry->addSignatures(content->signatures().mid(part->signatures().size()));
            auto c = new Entry(mNewModelPrivate);
            c->mData = mNewModelPrivate->getVar(content);
            _entry->addChild(c);
        }
//         foreach(const auto &content, part->availableContents()) {
//             foreach(const auto &contentPart, part->content(content)) {
//                 auto _entry = entry;
//                 _entry = _entry->addEncryptions(contentPart->encryptions().mid(part->encryptions().size()));
//                 _entry = _entry->addSignatures(contentPart->signatures().mid(part->signatures().size()));
//                 auto c = new Entry(mNewModelPrivate);
//                 c->mData = mNewModelPrivate->getVar(contentPart);
//                 _entry->addChild(c);
//             }
//         }
        foreach(const auto &sp, part->subParts()) {
            auto _entry = entry;
            _entry = _entry->addEncryptions(sp->encryptions().mid(part->encryptions().size()));
            _entry = _entry->addSignatures(sp->signatures().mid(part->signatures().size()));
            _entry->addPart(sp.get());
        }
        return entry;
    }

    int pos()
    {
        if(!mParent) {
            return -1;
        }
        int i=0;
        foreach(const auto &child, mParent->mChildren) {
            if (child == this) {
                return i;
            }
            i++;
        }
        return -1;
    }

    QSharedPointer<QVariant> mData;

    Entry *mParent;
    QVector<Entry *> mChildren;
    NewModelPrivate *mNewModelPrivate;
};


NewModelPrivate::NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<Parser> &parser)
    : q(q_ptr)
    , mRoot(std::unique_ptr<Entry>(new Entry(this)))
    , mParser(parser)
{
   mParts = mParser->collectContentParts();
   createTree();
}

NewModelPrivate::~NewModelPrivate()
{
}

void NewModelPrivate::createTree()
{
    auto root = mRoot.get();
    auto parent = root;
    Part *pPart = nullptr;
    QVector<Signature::Ptr> signatures;
    QVector<Encryption::Ptr> encryptions;
    foreach(const auto part, mParts) {
        auto _parent = parent;
        if (pPart != part->parent()) {
            auto _parent = root;
            _parent = _parent->addEncryptions(part->parent()->encryptions());
            _parent = _parent->addSignatures(part->parent()->signatures());
            signatures = part->parent()->signatures();
            encryptions = part->parent()->encryptions();
            parent = _parent;
            pPart = part->parent();
        }
        _parent = _parent->addEncryptions(part->encryptions().mid(encryptions.size()));
        _parent = _parent->addSignatures(part->signatures().mid(signatures.size()));
        _parent->addPart(part.get());
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
    return getVar(part.get());
}

QSharedPointer<QVariant> NewModelPrivate::getVar(Part *part)
{
    if (!mPartMap.contains(part)) {
        auto var = new QVariant();
        var->setValue(part);
        mPartMap.insert(part, QSharedPointer<QVariant>(var));
    }
    return mPartMap.value(part);
}

QSharedPointer<QVariant> NewModelPrivate::getVar(const std::shared_ptr<Content> &content)
{
    return getVar(content.get());
}

QSharedPointer<QVariant> NewModelPrivate::getVar(Content *content)
{
    if (!mCMap.contains(content)) {
        auto var = new QVariant();
        var->setValue(content);
        mCMap.insert(content, QSharedPointer<QVariant>(var));
    }
    return mCMap.value(content);
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

int NewModelPrivate::getPos(Content *content)
{
    int i = 0;
    foreach(const auto &c, content->parent()->content()) {
        if (c.get() == content) {
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
    Entry *entry = d->mRoot.get();
    if (parent.isValid()) {
        entry = static_cast<Entry *>(parent.internalPointer());
    }

    if (row < entry->mChildren.size()) {
        return createIndex(row, column, entry->mChildren.at(row));
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
        const auto entry = static_cast<Entry *>(index.internalPointer());
        const auto _data = entry->mData;
        if (entry == d->mRoot.get()|| !_data) {
            switch (role) {
            case Qt::DisplayRole:
                return QString("root");
            case IsEmbededRole:
                return false;
            }
            return QVariant();
        }
        if (_data->userType() ==  qMetaTypeId<Signature *>()) {
            const auto signature = _data->value<Signature *>();
            int i = d->getPos(signature);
            switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Signature%1").arg(i);
            case TypeRole:
                return QStringLiteral("Signature");
            case SecurityLevelRole:
                return QStringLiteral("RED");
            case IsEmbededRole:
                return data(index.parent(), IsEmbededRole);
            }
        } else if (_data->userType() ==  qMetaTypeId<Encryption *>()) {
            const auto encryption = _data->value<Encryption *>();
            int i = d->getPos(encryption);
            switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Encryption%1").arg(i);
            case TypeRole:
                return QStringLiteral("Encryption");
            case SecurityLevelRole:
                return QStringLiteral("GREEN");
            case IsEmbededRole:
                return data(index.parent(), IsEmbededRole);
            case EncryptionErrorType:
                {
                    switch(encryption->errorType()) {
                    case Encryption::NoError:
                        return QString();
                    case Encryption::PassphraseError:
                        return QStringLiteral("PassphraseError");
                    case Encryption::KeyMissing:
                        return QStringLiteral("KeyMissing");
                    default:
                        return QStringLiteral("UnknownError");
                    }
                }
            case EncryptionErrorString:
                return encryption->errorString();
            }
        } else if (_data->userType() ==  qMetaTypeId<Part *>()) {
            const auto part = _data->value<Part *>();
            switch (role) {
            case Qt::DisplayRole:
            case TypeRole:
                return QString::fromLatin1(part->type());
            case IsEmbededRole:
                return data(index.parent(), IsEmbededRole);
            }
        } else if (_data->userType() ==  qMetaTypeId<Content *>()) {
            const auto content = _data->value<Content *>();
            int i = d->getPos(content);
            switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Content%1").arg(i);
            case TypeRole:
                return QString::fromLatin1(content->type());
            case IsEmbededRole:
                return data(index.parent(), IsEmbededRole);
            case IsComplexHtmlContentRole: {
                const auto contentType = data(index, TypeRole).toString();
                if (contentType == "HtmlContent") {
                    const auto text = content->encodedContent();
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
                auto text = content->encodedContent();
                const auto contentType = data(index, TypeRole).toString();
                if (contentType == "HtmlContent") {
                    const auto rx = QRegExp("(src)\\s*=\\s*(\"|')(cid:[^\"']+)\\2");
                    int pos = 0;
                    while ((pos = rx.indexIn(text, pos)) != -1) {
                        const auto link = QUrl(rx.cap(3).toUtf8());
                        pos += rx.matchedLength();
                        const auto repl = d->mParser->getPart(link);
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
                } else { //We assume plain
                    //We alwas do richtext (so we get highlighted links and stuff).
                    return HtmlUtils::linkify(Qt::convertFromPlainText(text));
                }
                return text;
            }
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
    const auto entry = static_cast<Entry *>(index.internalPointer());
    if (entry->mParent && entry->mParent != d->mRoot.get()) {
        return createIndex(entry->pos(), 0, entry->mParent);
    }
    return QModelIndex();
}

int NewModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->mRoot->mChildren.size();
    } else {
        if (!parent.internalPointer()) {
            return 0;
        }
        const auto entry = static_cast<Entry *>(parent.internalPointer());
        return entry->mChildren.size();
    }
    return 0;
}

int NewModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}
