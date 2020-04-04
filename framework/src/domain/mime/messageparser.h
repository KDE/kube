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
#include "kube_export.h"
#include <QObject>
#include <QString>
#include <QStringList>

#include <QAbstractItemModel>
#include <QModelIndex>

#include <memory>

class QAbstractItemModel;

class MessagePartPrivate;

namespace MimeTreeParser {
    class ObjectTreeParser;
}

class KUBE_EXPORT MessageParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QVariant message READ message WRITE setMessage)
    Q_PROPERTY (QAbstractItemModel* parts READ parts NOTIFY htmlChanged)
    Q_PROPERTY (QAbstractItemModel* attachments READ attachments NOTIFY htmlChanged)
    Q_PROPERTY (QString rawContent READ rawContent NOTIFY htmlChanged)
    Q_PROPERTY (QString structureAsString READ structureAsString NOTIFY htmlChanged)
    Q_PROPERTY (bool loaded READ loaded NOTIFY htmlChanged)

public:
    explicit MessageParser(QObject *parent = Q_NULLPTR);
    ~MessageParser();

    QVariant message() const;
    void setMessage(const QVariant &to);
    QAbstractItemModel *parts() const;
    QAbstractItemModel *attachments() const;
    QString rawContent() const;
    QString structureAsString() const;
    bool loaded() const;
signals:
    void htmlChanged();

private:
    std::unique_ptr<MessagePartPrivate> d;
    QString mRawContent;
};

