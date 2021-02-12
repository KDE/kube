/*
    Copyright (c) 2021 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include "inboundmodel.h"

#include <QDebug>
#include <QStandardItem>
#include <QSettings>
#include <sink/store.h>
#include <sink/applicationdomaintype.h>

InboundModel::InboundModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    QByteArrayList roles{"type", "subtype", "timestamp", "message", "details", "entities", "resource", "data"};

    int role = Qt::UserRole + 1;
    mRoles.insert("id", role);
    role++;
    for (const auto &r : roles) {
        mRoles.insert(r, role);
        role++;
    }

    QHash<int, QByteArray> roleNames;
    for (const auto &r : mRoles.keys()) {
        roleNames.insert(mRoles[r], r);
    }

    mInboundModel = QSharedPointer<QStandardItemModel>::create();
    mInboundModel->setItemRoleNames(roleNames);
    setSourceModel(mInboundModel.data());

    setSortRole(mRoles.value("timestamp"));
    sort(0, Qt::DescendingOrder);

    init();
}

InboundModel::~InboundModel()
{

}

QString InboundModel::folderName(const QByteArray &id) const
{
    return mFolderNames.value(id);
}

void InboundModel::init()
{
    loadSettings();

    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    {

        Sink::Query folderQuery{};
        folderQuery.filter<Folder::Enabled>(true);
        folderQuery.request<Folder::SpecialPurpose>();
        folderQuery.request<Folder::Name>();

        Sink::Store::fetchAll<Folder>(folderQuery)
            .then([this] (const QList<Folder::Ptr> &list) {
                QList<QByteArray> folders;
                for (const auto &folder : list) {
                    const auto skip = [this, folder] {
                        for (const auto &purpose : folderSpecialPurposeBlacklist) {
                            if (folder->getSpecialPurpose().contains(purpose.toLatin1())) {
                                return true;
                            }
                        }
                        for (const auto &name : folderNameBlacklist) {
                            if (folder->getName().contains(name, Qt::CaseInsensitive)) {
                                return true;
                            }
                        }
                        return false;
                    }();
                    if (skip) {
                        continue;
                    }
                    folders << folder->identifier();
                    mFolderNames.insert(folder->identifier(), folder->getName());
                }

                Sink::Query query;
                query.setFlags(Sink::Query::LiveQuery);
                // query.resourceFilter<SinkResource::Account>(mCurrentAccount);
                query.sort<Mail::Date>();
                query.limit(100);
                query.reduce<ApplicationDomain::Mail::ThreadId>(Query::Reduce::Selector::max<ApplicationDomain::Mail::Date>())
                    .count()
                    .select<ApplicationDomain::Mail::Subject>(Query::Reduce::Selector::Min)
                    .collect<ApplicationDomain::Mail::Unread>()
                    .collect<ApplicationDomain::Mail::Important>();
                mSourceModel = Sink::Store::loadModel<Sink::ApplicationDomain::Mail>(query);
                QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsInserted, this, &InboundModel::mailRowsInserted);
            }).exec();
    }
}

void InboundModel::saveSettings()
{
    QSettings settings;
    settings.beginGroup("inbound");
    settings.setValue("senderBlacklist", QVariant::fromValue(QStringList{senderBlacklist.toList()}));
    settings.setValue("toBlacklist", QVariant::fromValue(QStringList{toBlacklist.toList()}));
    settings.setValue("folderSpecialPurposeBlacklist", QVariant::fromValue(QStringList{folderSpecialPurposeBlacklist}));

    for (auto it = perFolderMimeMessageWhitelistFilter.constBegin(); it != perFolderMimeMessageWhitelistFilter.constEnd(); it++) {
        settings.setValue("perFolderMimeMessageWhitelistFilter/" + it.key(), QVariant::fromValue(it.value()));
    }
    settings.setValue("folderNameBlacklist", QVariant::fromValue(QStringList{folderNameBlacklist}));
    settings.setValue("senderNameContainsFilter", QVariant::fromValue(senderNameContainsFilter));
}

void InboundModel::loadSettings()
{
    QSettings settings;
    settings.beginGroup("inbound");

    senderBlacklist = settings.value("senderBlacklist").toStringList().toSet();
    toBlacklist = settings.value("toBlacklist").toStringList().toSet();
    folderSpecialPurposeBlacklist = settings.value("folderSpecialPurposeBlacklist").toStringList();
    folderNameBlacklist = settings.value("folderNameBlacklist").toStringList();
    senderNameContainsFilter = settings.value("senderNameContainsFilter").toString();

    settings.beginGroup("perFolderMimeMessageWhitelistFilter");
    for (const auto &folder : settings.allKeys()) {
        perFolderMimeMessageWhitelistFilter.insert(folder, settings.value(folder).toString());
    }
}

bool InboundModel::filter(const Sink::ApplicationDomain::Mail &mail)
{
    if (!mFolderNames.contains(mail.getFolder())) {
        return true;
    }

    if (senderBlacklist.contains(mail.getSender().emailAddress)) {
        return true;
    }

    // Ignore own messages (phabricator only lists name)
    if (mail.getSender().name.contains(senderNameContainsFilter, Qt::CaseInsensitive)) {
        return true;
    }

    for (const auto &to : mail.getTo()) {
        if (toBlacklist.contains(to.emailAddress)) {
            return true;
        }
    }

    for (const auto &name : perFolderMimeMessageWhitelistFilter.keys()) {
        if (folderName(mail.getFolder()) == name) {
            //For this folder, exclude everything but what matches (whitelist)
            const auto &mimeMessage = mail.getMimeMessage();
            if (mimeMessage.contains(perFolderMimeMessageWhitelistFilter.value(name).toUtf8())) {
                return false;
            }
            return true;
        }
    }

    // Ignore thread if we have read all replies
    if (!mail.getCollectedProperty<Sink::ApplicationDomain::Mail::Unread>().contains(true)) {
        return true;
    }


    return false;
}

void InboundModel::add(const Sink::ApplicationDomain::Mail::Ptr &mail)
{
    if (filter(*mail)) {
        return;
    }
    insert({
        {"type", "mail"},
        {"message", QObject::tr("A new message is available: %1").arg(mail->getSubject())},
        {"subtype", "mail"},
        {"entities", QVariantList{mail->identifier()}},
        {"resource", QString{mail->resourceInstanceIdentifier()}},
        {"date", mail->getDate()},
        {"data", QVariantMap{
            {"subject", mail->getSubject()},
            {"unread", mail->getCollectedProperty<Sink::ApplicationDomain::Mail::Unread>().contains(true)},
            {"senderName", mail->getSender().name},
            {"folderName", folderName(mail->getFolder())},
            {"date", mail->getDate()},
            {"important", mail->getImportant()},
            {"trash", mail->getTrash()},
            {"threadSize", mail->count()},
            {"mail", QVariant::fromValue(mail)}
        }}
    });
}

void InboundModel::mailRowsInserted(const QModelIndex &parent, int first, int last)
{
    for (auto row = first; row <= last; row++) {
        auto entity = mSourceModel->index(row, 0, parent).data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
        add(entity);
    }
}

void InboundModel::insert(const QVariantMap &message)
{
    auto item = new QStandardItem;
    auto addProperty = [&] (const QByteArray &key) {
        item->setData(message.value(key), mRoles[key]);
    };
    item->setData(message.value("date"), mRoles["timestamp"]);
    addProperty("type");
    addProperty("subtype");
    addProperty("message");
    addProperty("details");
    addProperty("resource");
    addProperty("entities");
    addProperty("data");
    mInboundModel->insertRow(0, item);
    emit entryAdded(message);
}
