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
#include <memory>
#include <MimeTreeParser/MessagePart>

namespace MimeTreeParser {
    class NodeHelper;
};
class QAbstractItemModel;

class MessageParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QVariant message READ message WRITE setMessage)
    Q_PROPERTY (QString html READ html NOTIFY htmlChanged)
    Q_PROPERTY (QAbstractItemModel* partTree READ partTree NOTIFY htmlChanged)

public:
    explicit MessageParser(QObject *parent = Q_NULLPTR);

    QString html() const;

    QVariant message() const;
    void setMessage(const QVariant &to);
    QAbstractItemModel *partTree() const;

signals:
    void htmlChanged();

private:
    QSharedPointer<MimeTreeParser::MessagePart> mPartTree;
    QString mHtml;
    QMap<QByteArray, QUrl> mEmbeddedPartMap;
    std::shared_ptr<MimeTreeParser::NodeHelper> mNodeHelper;
};

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QDebug>

class PartModel : public QAbstractItemModel {
    Q_OBJECT
public:
    PartModel(QSharedPointer<MimeTreeParser::MessagePart> partTree, QMap<QByteArray, QUrl> embeddedPartMap);

public:
    enum Roles {
        Text  = Qt::UserRole + 1,
        IsHtml,
        IsEncrypted,
        IsAttachment,
        HasContent,
        Type
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
};

