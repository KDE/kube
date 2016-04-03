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
    std::shared_ptr<MimeTreeParser::NodeHelper> mNodeHelper;
};

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QDebug>

class PartModel : public QAbstractItemModel {
    Q_OBJECT
public:
    PartModel(QSharedPointer<MimeTreeParser::MessagePart> partTree) : mPartTree(partTree)
    {
    }

public:
    enum Roles {
        Text  = Qt::UserRole + 1,
        Type
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE
    {
        QHash<int, QByteArray> roles;
        roles[Text] = "text";
        roles[Type] = "type";
        return roles;
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE
    {
        qDebug() << "index " << parent << row << column << mPartTree->messageParts().size();
        if (!parent.isValid()) {
            if (row < mPartTree->messageParts().size()) {
                auto part = mPartTree->messageParts().at(row);
                return createIndex(row, column, part.data());
            }
        } else {
            auto part = static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer());
            auto subPart = part->messageParts().at(row);
            return createIndex(row, column, subPart.data());
        }
        return QModelIndex();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE
    {
        qDebug() << "Getting data for index";
        if (index.isValid()) {
            auto part = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
            switch (role) {
                case Text: {
                    QString text = part->property("htmlContent").toString();
                    text.truncate(fmin((text.indexOf(QStringLiteral("\n")) > -1) ? text.indexOf(QStringLiteral("\n")) : text.length(),100));
                    if (text.trimmed().isEmpty()) {
                        text = QStringLiteral("Empty");
                    }
                    return text;
                    break;
                }
                case Type:
                    return part->metaObject()->className();
            }
        }
        return QVariant();
    }

    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        qDebug() << "parent " << index;
        if (index.isValid()) {
            auto part = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
            auto parentPart = static_cast<MimeTreeParser::MessagePart*>(part->parentPart());
            auto row = 0;//get the parents parent to find the index
            if (!parentPart) {
                parentPart = mPartTree.data();
            }
            int i = 0;
            for (const auto &p : parentPart->messageParts()) {
                if (p.data() == part) {
                    row = i;
                    break;
                }
                i++;
            }
            return createIndex(row, index.column(), parentPart);
        }
        return QModelIndex();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE
    {
        qDebug() << "Row count " << parent;
        if (!parent.isValid()) {
            qDebug() << "Row count " << mPartTree->messageParts().size();
            return mPartTree->messageParts().size();
        } else {
            auto part = static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer());
            if (part) {
                return part->messageParts().size();
            }
        }
        return 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE
    {
        qDebug() << "Column count " << parent;
        return 1;
    }

private:
    QSharedPointer<MimeTreeParser::MessagePart> mPartTree;
};

