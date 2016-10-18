
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

class Entry
{
public:
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
};

class NewModelPrivate
{
public:
    NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<Parser> &parser);

    void createTree();
    Entry *addSignatures(Entry *parent, QVector<Signature::Ptr> signatures);
    Entry *addEncryptions(Entry *parent, QVector<Encryption::Ptr> encryptions);
    Entry *addPart(Entry *parent, Part *part);

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
    Entry *mRoot;

    std::shared_ptr<Parser> mParser;
    QMap<Part *, std::shared_ptr<NewContentModel>> mContentMap;
private:
    QMap<std::shared_ptr<Signature>, QSharedPointer<QVariant>> mSignatureMap;
    QMap<std::shared_ptr<Encryption>, QSharedPointer<QVariant>> mEncryptionMap;
    QMap<Part *, QSharedPointer<QVariant>> mPartMap;
    QMap<Content *, QSharedPointer<QVariant>> mCMap;
};

NewModelPrivate::NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<Parser> &parser)
    : q(q_ptr)
    , mParser(parser)
{
   mParts = mParser->collectContentParts();
   foreach(const auto &part, mParts) {
       mContentMap.insert(part.get(), std::shared_ptr<NewContentModel>(new NewContentModel(part, mParser)));
   }
   createTree();
}

Entry * NewModelPrivate::addSignatures(Entry *parent, QVector<Signature::Ptr> signatures)
{
    auto ret = parent;
    foreach(const auto &sig, signatures) {
        auto entry = new Entry();
        entry->mData = getVar(sig);
        ret = entry;
        parent->addChild(entry);
    }
    return ret;
}

Entry * NewModelPrivate::addEncryptions(Entry *parent, QVector<Encryption::Ptr> encryptions)
{
    auto ret = parent;
    foreach(const auto &enc, encryptions) {
        auto entry = new Entry();
        entry->mData = getVar(enc);
        parent->addChild(entry);
        ret = entry;
    }
    return ret;
}

Entry * NewModelPrivate::addPart(Entry *parent, Part *part)
{
    auto entry = new Entry();
    entry->mData = getVar(part);
    parent->addChild(entry);

    foreach(const auto &content, part->content()) {
        auto _entry = entry;
        _entry = addSignatures(_entry, content->signatures().mid(part->signatures().size()));
        _entry = addEncryptions(_entry, content->encryptions().mid(part->encryptions().size()));
        auto c = new Entry();
        c->mData = getVar(content);
        _entry->addChild(c);
    }

    foreach(const auto &sp, part->subParts()) {
        auto _entry = entry;
        _entry = addSignatures(_entry, sp->signatures().mid(part->signatures().size()));
        _entry = addEncryptions(_entry, sp->encryptions().mid(part->encryptions().size()));
        addPart(_entry, sp.get());
    }
    return entry;
}

void NewModelPrivate::createTree()
{
    mRoot = new Entry();
    auto parent = mRoot;
    Part *pPart = nullptr;
    QVector<Signature::Ptr> signatures;
    QVector<Encryption::Ptr> encryptions;
    foreach(const auto part, mParts) {
        auto _parent = parent;
        if (pPart != part->parent()) {
            auto _parent = mRoot;
            _parent = addSignatures(_parent, part->parent()->signatures());
            _parent = addEncryptions(_parent, part->parent()->encryptions());
            signatures = part->parent()->signatures();
            encryptions = part->parent()->encryptions();
            parent = _parent;
            pPart = part->parent();
        }
        _parent = addSignatures(_parent, part->signatures().mid(signatures.size()));
        _parent = addEncryptions(_parent, part->encryptions().mid(encryptions.size()));
        addPart(_parent, part.get());
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
    roles[ContentsRole] = "contents";
    roles[ContentRole] = "content";
    roles[IsEmbededRole] = "embeded";
    roles[SecurityLevelRole] = "securityLevel";
    return roles;
}


QModelIndex NewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }
    Entry *entry = d->mRoot;
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
        if (role == Qt::DisplayRole) {
            return QString("root");
        }
        return QVariant();
    }
    if (index.internalPointer()) {
        const auto entry = static_cast<Entry *>(index.internalPointer());
        const auto _data = entry->mData;
        if (_data->userType() ==  qMetaTypeId<Signature *>()) {
            const auto signature = _data->value<Signature *>();
            int i = d->getPos(signature);
            switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Signature%1").arg(i);
            case TypeRole:
                return QStringLiteral("Signature");
            }
        } else if (_data->userType() ==  qMetaTypeId<Encryption *>()) {
            const auto encryption = _data->value<Encryption *>();
            int i = d->getPos(encryption);
            switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Encryption%1").arg(i);
            case TypeRole:
                return QStringLiteral("Encryption");
            }
        } else if (_data->userType() ==  qMetaTypeId<Part *>()) {
            const auto part = _data->value<Part *>();
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
        } else if (_data->userType() ==  qMetaTypeId<Content *>()) {
            const auto content = _data->value<Content *>();
            int i = d->getPos(content);
            switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Content%1").arg(i);
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
    if (entry->mParent) {
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
