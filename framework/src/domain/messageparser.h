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

class QAbstractItemModel;

class Parser;
class MessagePartPrivate;

class NewModelPrivate;
class AttachmentModelPrivate;

class MessageParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QVariant message READ message WRITE setMessage)
    Q_PROPERTY (QAbstractItemModel* newTree READ newTree NOTIFY htmlChanged)
    Q_PROPERTY (QAbstractItemModel* attachments READ attachments NOTIFY htmlChanged)

public:
    explicit MessageParser(QObject *parent = Q_NULLPTR);
    ~MessageParser();

    QVariant message() const;
    void setMessage(const QVariant &to);
    QAbstractItemModel *newTree() const;
    QAbstractItemModel *attachments() const;

signals:
    void htmlChanged();

private:
    std::unique_ptr<MessagePartPrivate> d;
};

class NewModel : public QAbstractItemModel {
    Q_OBJECT
public:
    NewModel(std::shared_ptr<Parser> parser);
    ~NewModel();

public:
    enum Roles {
        TypeRole  = Qt::UserRole + 1,
        ContentsRole,
        ContentRole,
        IsEmbededRole,
        SecurityLevelRole,
        EncryptionErrorType,
        EncryptionErrorString
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    std::unique_ptr<NewModelPrivate> d;
};

class AttachmentModel : public QAbstractItemModel {
     Q_OBJECT
public:
    AttachmentModel(std::shared_ptr<Parser> parser);
    ~AttachmentModel();

public:
    enum Roles {
        TypeRole  = Qt::UserRole + 1,
        IconRole,
        NameRole,
        SizeRole,
        IsEncryptedRole,
        IsSignedRole
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    std::unique_ptr<AttachmentModelPrivate> d;
};
