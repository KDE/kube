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
    : QSortFilterProxyModel(parent),
    mMinNumberOfItems{50}
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

void InboundModel::ignoreSender(const QVariant &variant)
{
    if (auto mail = variant.value<Sink::ApplicationDomain::Mail::Ptr>()) {
        const auto sender = mail->getSender().emailAddress;
        qDebug() << "Ignoring " << sender;
        senderBlacklist.insert(sender);
        saveSettings();
        refresh(true, false);
    }
}

QString InboundModel::folderName(const QByteArray &id) const
{
    return mFolderNames.value(id);
}

void InboundModel::refresh()
{
    refresh(true, true);
}

void InboundModel::refresh(bool refreshMail, bool refreshCalendar)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    mFolderNames.clear();
    mEventsLoaded = false;

    loadSettings();

    if (refreshMail) {
        removeAllByType("mail");

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
                query.limit(mMinNumberOfItems);
                query.reduce<ApplicationDomain::Mail::ThreadId>(Query::Reduce::Selector::max<ApplicationDomain::Mail::Date>())
                    .count()
                    .select<ApplicationDomain::Mail::Subject>(Query::Reduce::Selector::Min)
                    .collect<ApplicationDomain::Mail::Unread>()
                    .collect<ApplicationDomain::Mail::Important>();

                query.setPostQueryFilter([=, folderNames = mFolderNames] (const ApplicationDomain::ApplicationDomainType &entity){
                    const ApplicationDomain::Mail mail(entity);
                    //TODO turn into query filter
                    if (!folderNames.contains(mail.getFolder())) {
                        return false;
                    }

                    if (senderBlacklist.contains(mail.getSender().emailAddress)) {
                        return false;
                    }

                    // Ignore own messages (phabricator only lists name)
                    if (!senderNameContainsFilter.isEmpty() && mail.getSender().name.contains(senderNameContainsFilter, Qt::CaseInsensitive)) {
                        return false;
                    }

                    for (const auto &to : mail.getTo()) {
                        if (toBlacklist.contains(to.emailAddress)) {
                            return false;
                        }
                    }

                    const auto &mimeMessage = mail.getMimeMessage();

                    for (const auto &filter : messageFilter) {
                        if (filter.match(mimeMessage).hasMatch()) {
                            return false;
                        }
                    }

                    for (const auto &name : perFolderMimeMessageWhitelistFilter.keys()) {
                        if (folderNames.value(mail.getFolder()) == name) {
                            //For this folder, exclude everything but what matches (whitelist)
                            return mimeMessage.contains(perFolderMimeMessageWhitelistFilter.value(name).toUtf8());
                        }
                    }
                    return true;
                });

                mSourceModel = Sink::Store::loadModel<Sink::ApplicationDomain::Mail>(query);
                QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsInserted, this, &InboundModel::mailRowsInserted);
                QObject::connect(mSourceModel.data(), &QAbstractItemModel::dataChanged, this, &InboundModel::mailDataChanged);
                QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsRemoved, this, &InboundModel::mailRowsRemoved);

                QObject::connect(mSourceModel.data(), &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &roles) {
                    if (roles.contains(Sink::Store::ChildrenFetchedRole)) {
                        if (mEventsLoaded) {
                            emit initialItemsLoaded();
                        }
                    }
                });
            }).exec();
    }

    if (refreshCalendar) {
        removeAllByType("event");
        Sink::Query calendarQuery{};
        calendarQuery.filter<Calendar::Enabled>(true);
        calendarQuery.request<Calendar::Name>();

        Sink::Store::fetchAll<Calendar>(calendarQuery)
            .then([this] (const QList<Calendar::Ptr> &list) {

                QList<QString> calendarFilter;
                for (const auto &calendar : list) {
                    calendarFilter << QString{calendar->identifier()};
                }

                auto model = QSharedPointer<EventOccurrenceModel>::create();
                model->setStart(mCurrentDateTime.date());
                model->setLength(7);
                model->setCalendarFilter(calendarFilter);

                mEventSourceModel = model;
                QObject::connect(mEventSourceModel.data(), &QAbstractItemModel::rowsInserted, this, &InboundModel::eventRowsInserted);
                QObject::connect(mEventSourceModel.data(), &QAbstractItemModel::modelReset, this, &InboundModel::eventModelReset);
            }).exec();
    }
}

int InboundModel::firstRecentIndex()
{
    // qWarning() << "Getting first recent index" << index.row();
    for (const auto  &index : match(index(0, 0), mRoles["type"], "mail", 1, Qt::MatchExactly)) {
        qWarning() << "First recent index" << index.row();
        return index.row();
    }
    return 0;
}

void InboundModel::init()
{
    refresh();
}

void InboundModel::configure(
    const QSet<QString> &_senderBlacklist,
    const QSet<QString> &_toBlacklist,
    const QString &_senderNameContainsFilter,
    const QMap<QString, QString> &_perFolderMimeMessageWhitelistFilter,
    const QList<QRegularExpression> &_messageFilter,
    const QList<QString> &_folderSpecialPurposeBlacklist,
    const QList<QString> &_folderNameBlacklist
)
{
    senderBlacklist = _senderBlacklist;
    toBlacklist = _toBlacklist;
    senderNameContainsFilter = _senderNameContainsFilter;
    perFolderMimeMessageWhitelistFilter = _perFolderMimeMessageWhitelistFilter;
    messageFilter = _messageFilter;
    folderSpecialPurposeBlacklist = _folderSpecialPurposeBlacklist;
    folderNameBlacklist = _folderSpecialPurposeBlacklist;

    saveSettings();
    init();
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

    messageFilter.clear();
    for (const auto filter : settings.value("messageFilter").toStringList()) {
        messageFilter.append(QRegularExpression{filter});
    }

    settings.beginGroup("perFolderMimeMessageWhitelistFilter");
    perFolderMimeMessageWhitelistFilter.clear();
    for (const auto &folder : settings.allKeys()) {
        perFolderMimeMessageWhitelistFilter.insert(folder, settings.value(folder).toString());
    }
}

QVariantMap InboundModel::toVariantMap(const Sink::ApplicationDomain::Mail::Ptr &mail)
{
    return {
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
            {"mail", QVariant::fromValue(mail)},
            {"domainObject", QVariant::fromValue(mail)}
        }
        }
    };
}

void InboundModel::add(const Sink::ApplicationDomain::Mail::Ptr &mail)
{
    insert(mail->identifier(), toVariantMap(mail));
}

void InboundModel::update(const Sink::ApplicationDomain::Mail::Ptr &mail)
{
    update(mail->identifier(), toVariantMap(mail));
}

void InboundModel::remove(const Sink::ApplicationDomain::Mail::Ptr &mail)
{
    for (auto item : mInboundModel->findItems(QString{mail->identifier()})) {
        mInboundModel->removeRows(item->row(), 1);
    }
}

void InboundModel::mailRowsInserted(const QModelIndex &parent, int first, int last)
{
    for (auto row = first; row <= last; row++) {
        auto entity = mSourceModel->index(row, 0, parent).data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
        add(entity);
    }
}

void InboundModel::mailRowsRemoved(const QModelIndex &parent, int first, int last)
{
    for (auto row = first; row <= last; row++) {
        auto entity = mSourceModel->index(row, 0, parent).data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
        remove(entity);
    }
}

void InboundModel::mailDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (!topLeft.isValid() || !bottomRight.isValid()) {
        return;
    }
    for (auto row = topLeft.row(); row <= bottomRight.row(); row++) {
        auto entity = mSourceModel->index(row, 0, topLeft.parent()).data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
        update(entity);
    }
}


bool InboundModel::filter(const EventOccurrenceModel::Occurrence &occurrence)
{
    //Filter todays events that have already passed
    if (occurrence.start < mCurrentDateTime) {
        return true;
    }

    return false;
}

void InboundModel::eventRowsInserted(const QModelIndex &parent, int first, int last)
{
    for (auto row = first; row <= last; row++) {
        auto idx = mEventSourceModel->index(row, 0, parent);
        auto event = idx.data(EventOccurrenceModel::Event).value<Sink::ApplicationDomain::Event::Ptr>();
        auto occurrence = idx.data(EventOccurrenceModel::EventOccurrence).value<EventOccurrenceModel::Occurrence>();

        if (filter(occurrence)) {
            continue;
        }

        const QVariantMap variantMap {
            {"type", "event"},
            {"message", QObject::tr("A new event is available: %1").arg(event->getSummary())},
            {"subtype", "event"},
            {"entities", QVariantList{event->identifier()}},
            {"resource", QString{event->resourceInstanceIdentifier()}},
            {"date", occurrence.start},
            {"data", QVariantMap{
                {"subject", event->getSummary()},
                {"domainObject", QVariant::fromValue(event)},
                {"occurrence", QVariant::fromValue(occurrence)},
                {"date", occurrence.start},
            }
            }
        };

        insert(event->identifier(), variantMap);
    }
}

void InboundModel::getAllByType(const QString &type, std::function<QModelIndex(const QModelIndex &)> callback)
{
    QModelIndex index = mInboundModel->index(0, 0);
    while (true) {
        const auto list = mInboundModel->match(index, mRoles["type"], type, 1, Qt::MatchExactly);
        if (list.isEmpty()) {
            break;
        }
        index = callback(list.first());
    }
}

void InboundModel::removeAllByType(const QString &type)
{
    getAllByType(type, [this](const QModelIndex &index) {
        mInboundModel->removeRow(index.row());
        return mInboundModel->index(0, 0);
    });
}

void InboundModel::eventModelReset()
{
    removeAllByType("event");
    eventRowsInserted({}, 0, mEventSourceModel->rowCount() - 1);
    mEventsLoaded = true;
    if (mSourceModel->data({}, Sink::Store::ChildrenFetchedRole).toBool()) {
        emit initialItemsLoaded();
    }
}

void InboundModel::insert(const QByteArray &key, const QVariantMap &message)
{
    auto item = new QStandardItem{QString{key}};
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

void InboundModel::update(const QByteArray &key, const QVariantMap &message)
{
    for (auto item : mInboundModel->findItems(QString{key})) {
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
    }
}

void InboundModel::setCurrentDate(const QDateTime &dt)
{
    mCurrentDateTime = dt;
    getAllByType("event", [&](const QModelIndex &index) {
        const auto occurrence = index.data(mRoles["data"]).toMap()["occurrence"].value<EventOccurrenceModel::Occurrence>();
        if (filter(occurrence)) {
            mInboundModel->removeRow(index.row());
        }
        return index.sibling(index.row() + 1, index.column());
    });
}
