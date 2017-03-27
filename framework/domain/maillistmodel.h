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

#include <sink/store.h>

#include <QSortFilterProxyModel>
#include <QSharedPointer>
#include <QStringList>

class MailListModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY (QVariant parentFolder READ parentFolder WRITE setParentFolder)
    Q_PROPERTY (QVariant mail READ mail WRITE setMail)
    Q_PROPERTY (QString filter READ filter WRITE setFilter)

public:
    enum Status {
        NoStatus,
        InProgressStatus,
        ErrorStatus
    };
    Q_ENUMS(Status)

    MailListModel(QObject *parent = Q_NULLPTR);
    ~MailListModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;

    enum Roles {
        Subject  = Qt::UserRole + 1,
        Sender,
        SenderName,
        To,
        Cc,
        Bcc,
        Date,
        Unread,
        Important,
        Draft,
        Sent,
        Trash,
        Id,
        MimeMessage,
        DomainObject,
        ThreadSize,
        Mail,
        Incomplete,
        Status
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    void runQuery(const Sink::Query &query);

    void setParentFolder(const QVariant &parentFolder);
    QVariant parentFolder() const;

    void setMail(const QVariant &mail);
    QVariant mail() const;

    void setFilter(const QString &mail);
    QString filter() const;

private:
    void fetchMail(Sink::ApplicationDomain::Mail::Ptr mail);

    QSharedPointer<QAbstractItemModel> m_model;
    bool mFetchMails = false;
    QSet<QByteArray> mFetchedMails;
    QByteArray mCurrentQueryItem;
};
