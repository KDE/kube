/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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

#include "folderlistmodel.h"

#include <QObject>
#include <QScopedPointer>
#include <QString>

class FolderListController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString accountId READ accountId WRITE setAccountId NOTIFY accountIdChanged)
    Q_PROPERTY (FolderListModel *model READ model CONSTANT)

public:
    explicit FolderListController(QObject *parent = Q_NULLPTR);

    QString accountId() const;
    void setAccountId(const QString &id);

    FolderListModel *model() const;

    void loadFolders(const QString &id);

signals:
    void accountIdChanged();

public slots:
    void deleteFolder(const QString &id);
    void addFolder(const QString &name);


private:
    QString m_accountId;
    QScopedPointer<FolderListModel> m_model;
};
