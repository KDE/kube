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

#include <QAbstractItemModel>
#include <QModelIndex>

#include <memory>

namespace MimeTreeParser {
    class ObjectTreeParser;
}
class AttachmentModelPrivate;

class AttachmentModel : public QAbstractItemModel {
     Q_OBJECT
public:
    AttachmentModel(std::shared_ptr<MimeTreeParser::ObjectTreeParser> parser);
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

    Q_INVOKABLE bool saveAttachmentToDisk(const QModelIndex &parent);
    Q_INVOKABLE bool openAttachment(const QModelIndex &index);

    Q_INVOKABLE bool importPublicKey(const QModelIndex &index);

private:
    std::unique_ptr<AttachmentModelPrivate> d;
};
