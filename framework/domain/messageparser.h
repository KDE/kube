/*
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

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <QAbstractItemModel>
#include <QModelIndex>

#include <memory>
#include <MimeTreeParser/MessagePart>

class QAbstractItemModel;

class Parser;
class Part;
typedef std::shared_ptr<Part> PartPtr;
class Content;
typedef std::shared_ptr<Content> ContentPtr;
class MessagePartPrivate;

class MessageParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QVariant message READ message WRITE setMessage)
    Q_PROPERTY (QString html READ html NOTIFY htmlChanged)
    Q_PROPERTY (QAbstractItemModel* partTree READ partTree NOTIFY htmlChanged)
    Q_PROPERTY (QAbstractItemModel* newTree READ newTree NOTIFY htmlChanged)

public:
    explicit MessageParser(QObject *parent = Q_NULLPTR);
    ~MessageParser();

    QString html() const;

    QVariant message() const;
    void setMessage(const QVariant &to);
    QAbstractItemModel *partTree() const;
    QAbstractItemModel *newTree() const;

signals:
    void htmlChanged();

private:
    std::unique_ptr<MessagePartPrivate> d;
};

class PartModel : public QAbstractItemModel {
    Q_OBJECT
public:
    PartModel(QSharedPointer<MimeTreeParser::MessagePart> partTree, std::shared_ptr<Parser> parser);

public:
    enum Roles {
        Text  = Qt::UserRole + 1,
        IsHtml,
        IsEncrypted,
        IsAttachment,
        HasContent,
        Type,
        IsHidden
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    QSharedPointer<MimeTreeParser::MessagePart> mPartTree;
    QMap<QByteArray, QUrl> mEmbeddedPartMap;
    std::shared_ptr<Parser> mParser;
};


class NewContentModel : public QAbstractItemModel {
    Q_OBJECT
public:
    NewContentModel (const PartPtr &part);

public:
    enum Roles {
        TypeRole  = Qt::UserRole + 1,
        ContentRole,
        IsEmbededRole,
        SecurityLevelRole
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    const PartPtr &mPart;
};

class NewModel : public QAbstractItemModel {
    Q_OBJECT
public:
    NewModel(std::shared_ptr<Parser> parser);

public:
    enum Roles {
        TypeRole  = Qt::UserRole + 1,
        ContentsRole,
        IsEmbededRole,
        SecurityLevelRole
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    std::shared_ptr<Parser> mParser;
    QVector<PartPtr> mParts;
    QMap<Part *, std::shared_ptr<NewContentModel>> mContentMap;
};

