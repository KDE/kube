
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

NewModel::NewModel(std::shared_ptr<Parser> parser)
    : mParser(parser)
{
   mParts = mParser->collectContentParts();
   foreach(const auto &part, mParts) {
       mContentMap.insert(part.get(), std::shared_ptr<NewContentModel>(new NewContentModel(part, mParser)));
   }
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
    if (!parent.isValid()) {
        if (row < mParts.size()) {
            auto part = mParts.at(row);
            return createIndex(row, column, part.get());
        }
    }
    return QModelIndex();
}

QVariant NewModel::data(const QModelIndex &index, int role) const
{
    if (!index.parent().isValid()) {
        auto part = static_cast<Part *>(index.internalPointer());
        switch (role) {
            case TypeRole:
                return QString::fromLatin1(part->type());
            case IsEmbededRole:
                return index.parent().isValid();
            case SecurityLevelRole:
                return QStringLiteral("GRAY");
            case ContentsRole:
                return  QVariant::fromValue<QAbstractItemModel *>(mContentMap.value(part).get());
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
    if (!parent.isValid()) {
        return mParts.size();
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
