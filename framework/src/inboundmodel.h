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
#include "kube_export.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QVariantMap>
#include <QSet>
#include <QRegularExpression>
#include <QString>
#include <QDateTime>
#include <fabric.h>

#include "eventoccurrencemodel.h"

namespace Sink {
    namespace ApplicationDomain {
        class Mail;
    };
};

class KUBE_EXPORT InboundModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QDateTime currentDate MEMBER mCurrentDateTime)

public:
    InboundModel(QObject *parent = Q_NULLPTR);
    ~InboundModel();

    Q_INVOKABLE void insert(const QByteArray &key, const QVariantMap &);
    Q_INVOKABLE void update(const QByteArray &key, const QVariantMap &);
    Q_INVOKABLE void refresh();

signals:
    void entryAdded(const QVariantMap &message);

private slots:
    void mailRowsInserted(const QModelIndex &parent, int first, int last);
    void mailRowsRemoved(const QModelIndex &parent, int first, int last);
    void mailDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void eventRowsInserted(const QModelIndex &parent, int first, int last);
    void eventModelReset();

private:
    void saveSettings();
    void loadSettings();
    void init();
    void add(const QSharedPointer<Sink::ApplicationDomain::Mail> &);
    void remove(const QSharedPointer<Sink::ApplicationDomain::Mail> &);
    void update(const QSharedPointer<Sink::ApplicationDomain::Mail> &);
    QVariantMap toVariantMap(const QSharedPointer<Sink::ApplicationDomain::Mail> &mail);
    QString folderName(const QByteArray &id) const;
    bool filter(const Sink::ApplicationDomain::Mail &mail);
    bool filter(const EventOccurrenceModel::Occurrence &mail);

    QHash<QByteArray, int> mRoles;
    QHash<QByteArray, QString> mFolderNames;
    QSharedPointer<QAbstractItemModel> mSourceModel;
    QSharedPointer<QAbstractItemModel> mEventSourceModel;
    QSharedPointer<QStandardItemModel> mInboundModel;
    QSet<QString> senderBlacklist;
    QSet<QString> toBlacklist;
    QString senderNameContainsFilter;
    QMap<QString, QString> perFolderMimeMessageWhitelistFilter;
    QList<QRegularExpression> messageFilter;
    QList<QString> folderSpecialPurposeBlacklist;
    QList<QString> folderNameBlacklist;
    QDateTime mCurrentDateTime;
};
