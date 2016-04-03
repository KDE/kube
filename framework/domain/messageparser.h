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
    QSharedPointer<MimeTreeParser::MessagePartList> mPartTree;
    QString mHtml;
    std::shared_ptr<MimeTreeParser::NodeHelper> mNodeHelper;
};

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QDebug>

class PartModel : public QAbstractItemModel {
    Q_OBJECT
public:
    PartModel(QSharedPointer<MimeTreeParser::MessagePartList> partTree) : mPartTree(partTree)
    {
    }

public:
    enum Roles {
        Text  = Qt::UserRole + 1,
        IsHtml,
        IsEncrypted,
        IsAttachment,
        HasContent
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE
    {
        QHash<int, QByteArray> roles;
        roles[Text] = "text";
        roles[IsHtml] = "isHtml";
        roles[IsEncrypted] = "isEncrypted";
        roles[IsAttachment] = "isAttachment";
        roles[HasContent] = "hasContent";
        return roles;
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE
    {
        qDebug() << "index " << parent << row << column << mPartTree->messageParts().size();
        if (!parent.isValid()) {
            if (row < mPartTree->messageParts().size()) {
                auto part = mPartTree->messageParts().at(row);
                qDebug() << "creating index " << row;
                // qDebug() << "part text: " << part->property("text").toString();
                return createIndex(row, column, part.data());
            }
        } else {
            auto part = static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer());
            auto subListPart = part->subMessagePart();
            if (subListPart) {
                auto subPart = subListPart->messageParts().at(row);
                return createIndex(row, column, subPart.data());
            }
        }
        return QModelIndex();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE
    {
        qDebug() << "Getting data for index";
        if (index.isValid()) {
            auto part = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
            switch (role) {
                case Text:
                    // qDebug() << "Getting text: " << part->property("text").toString();
                    return part->property("htmlContent").toString();
                case IsAttachment:
                    return part->property("attachment").toBool();
                case IsEncrypted:
                    return part->property("isEncrypted").toBool();
                case IsHtml:
                    return part->property("isHtml").toBool();
                case HasContent:
                    return !part->property("htmlContent").toString().isEmpty();
            }
        }
        return QVariant();
    }

    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        qDebug() << "parent " << index;
        if (index.isValid()) {
            auto part = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
            auto parentListPart = part->parentPart();
            if (parentListPart) {
                auto parentPart = parentListPart->parentPart();
                if (parentPart) {
                    auto row = 0;//get the parents parent to find the index
                    MimeTreeParser::MessagePartList *parentList;
                    if (parentPart->parentPart()) {
                        parentList = static_cast<MimeTreeParser::MessagePartList*>(parentPart->parentPart());
                    } else {
                        parentList = mPartTree.data();
                    }
                    int i = 0;
                    for (const auto &p : parentList->messageParts()) {
                        if (p.data() == parentPart) {
                            row = i;
                            break;
                        }
                        i++;
                    }
                    return createIndex(row, index.column(), parentPart);
                }
            }
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
            auto subListPart = part->subMessagePart();
            if (subListPart) {
                return subListPart->messageParts().size();
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
    QSharedPointer<MimeTreeParser::MessagePartList> mPartTree;
};

