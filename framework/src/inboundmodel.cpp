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
#include <sink/standardqueries.h>

InboundModel::InboundModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    mMinNumberOfItems{50}
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);

    QByteArrayList roles{"type", "subtype", "timestamp", "message", "details", "entities", "resource", "data"};

    int role = Qt::UserRole + 1;
    mRoles.insert("id", role);
    role++;
    for (const auto &r : roles) {
        mRoles.insert(r, role);
        role++;
    }

    for (const auto &r : mRoles.keys()) {
        mRoleNames.insert(mRoles[r], r);
    }
}

InboundModel::~InboundModel()
{

}

static QByteArray getIdentifier(const QVariant &v)
{
    if (v.canConvert<Sink::ApplicationDomain::Event::Ptr>()) {
        return v.value<Sink::ApplicationDomain::Event::Ptr>()->identifier();
    }
    if (v.canConvert<Sink::ApplicationDomain::Mail::Ptr>()) {
        return v.value<Sink::ApplicationDomain::Mail::Ptr>()->identifier();
    }
    return {};
}

static QDateTime getDate(const QVariant &v)
{
    if (v.canConvert<Sink::ApplicationDomain::Event::Ptr>()) {
        return v.value<Sink::ApplicationDomain::Event::Ptr>()->getStartTime();
    }
    if (v.canConvert<Sink::ApplicationDomain::Mail::Ptr>()) {
        return v.value<Sink::ApplicationDomain::Mail::Ptr>()->getDate();
    }
    return {};
}

bool InboundModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (!m_model) {
        const auto leftDate = left.data(mRoles["timestamp"]).toDateTime();
        const auto rightDate = right.data(mRoles["timestamp"]).toDateTime();
        if (leftDate == rightDate) {
            return left.data(mRoles["id"]).toByteArray() <
                    right.data(mRoles["id"]).toByteArray();
        }
        return leftDate < rightDate;
    }
    const auto leftDate = getDate(left.data(Sink::Store::DomainObjectRole));
    const auto rightDate = getDate(right.data(Sink::Store::DomainObjectRole));
    if (leftDate == rightDate) {
        return getIdentifier(left.data(Sink::Store::DomainObjectRole)) <
                getIdentifier(right.data(Sink::Store::DomainObjectRole));
    }
    return leftDate < rightDate;
}

void InboundModel::ignoreSender(const QVariant &variant)
{
    if (auto mail = variant.value<Sink::ApplicationDomain::Mail::Ptr>()) {
        const auto sender = mail->getSender().emailAddress;
        qDebug() << "Ignoring " << sender;
        senderBlacklist.insert(sender);
        saveSettings();
        refreshMail();
    }
}

QString InboundModel::folderName(const QByteArray &id) const
{
    return mFolderNames.value(id);
}

static bool applyStringFilter(Sink::Query &query, const QString &filterString)
{
    if (filterString.length() < 3 && !filterString.isEmpty()) {
        return false;
    }
    if (!filterString.isEmpty()) {
        query.filter({}, Sink::QueryBase::Comparator(filterString, Sink::QueryBase::Comparator::Fulltext));
        query.limit(0);
    }
    return true;
}


void InboundModel::refresh()
{
    if (mCurrentDateTime.isValid() && mInboundModel) {
        refreshMail();
        refreshCalendar();
    }
}

void InboundModel::refreshMail()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    mFolderNames.clear();

    loadSettings();

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
            query.filter<Sink::ApplicationDomain::Mail::Trash>(false);
            query.reduce<ApplicationDomain::Mail::ThreadId>(Query::Reduce::Selector::max<ApplicationDomain::Mail::Date>())
                .count()
                .select<ApplicationDomain::Mail::Subject>(Query::Reduce::Selector::Min)
                .collect<ApplicationDomain::Mail::Unread>()
                .collect<ApplicationDomain::Mail::Important>();


            if (mFilter.value("string").isValid()) {
                applyStringFilter(query, mFilter.value("string").toString());
            }

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
            QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &InboundModel::mailRowsRemoved);

            QObject::connect(mSourceModel.data(), &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &roles) {
                if (roles.contains(Sink::Store::ChildrenFetchedRole)) {
                    //Only emit initialItemsLoaded if both source models have finished loading
                    if (mEventSourceModel && static_cast<EventOccurrenceModel*>(mEventSourceModel.data())->initialItemsComplete()) {
                        emit initialItemsLoaded();
                    }
                }
            });
        }).exec();
}

void InboundModel::refreshCalendar()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    loadSettings();

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
            Q_ASSERT(mCurrentDateTime.isValid());
            model->setStart(mCurrentDateTime.date());
            model->setLength(7);
            model->setCalendarFilter(calendarFilter);
            //TODO text filter

            mEventSourceModel = model;
            QObject::connect(mEventSourceModel.data(), &QAbstractItemModel::rowsInserted, this, &InboundModel::eventRowsInserted);
            QObject::connect(mEventSourceModel.data(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &InboundModel::eventRowsRemoved);
            QObject::connect(mEventSourceModel.data(), &QAbstractItemModel::dataChanged, this, &InboundModel::eventDataChanged);
            QObject::connect(model.data(), &EventOccurrenceModel::initialItemsLoaded, this, [this]() {
                //Only emit initialItemsLoaded if both source models have finished loading
                if (mSourceModel->data({}, Sink::Store::ChildrenFetchedRole).toBool()) {
                    emit initialItemsLoaded();
                }
            });
        }).exec();
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

void InboundModel::initInboundFilter()
{
    mInboundModel = QSharedPointer<QStandardItemModel>::create();
    mInboundModel->setItemRoleNames(mRoleNames);
    setSourceModel(mInboundModel.data());
    refresh();
}

void InboundModel::configure(
    const QSet<QString> &_senderBlacklist,
    const QSet<QString> &_toBlacklist,
    const QString &_senderNameContainsFilter,
    const QMap<QString, QString> &_perFolderMimeMessageWhitelistFilter,
    const QList<QRegularExpression> &_messageFilter,
    const QList<QString> &_folderSpecialPurposeBlacklist,
    const QList<QString> &/*_folderNameBlacklist*/
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
    initInboundFilter();
}

void InboundModel::saveSettings()
{
    QSettings settings;
    settings.beginGroup("inbound");
    settings.setValue("senderBlacklist", QVariant::fromValue(QStringList{senderBlacklist.values()}));
    settings.setValue("toBlacklist", QVariant::fromValue(QStringList{toBlacklist.values()}));
    settings.setValue("folderSpecialPurposeBlacklist", QVariant::fromValue(QStringList{folderSpecialPurposeBlacklist}));

    for (auto it = perFolderMimeMessageWhitelistFilter.constBegin(); it != perFolderMimeMessageWhitelistFilter.constEnd(); it++) {
        settings.setValue("perFolderMimeMessageWhitelistFilter/" + it.key(), QVariant::fromValue(it.value()));
    }
    settings.setValue("folderNameBlacklist", QVariant::fromValue(QStringList{folderNameBlacklist}));
    settings.setValue("senderNameContainsFilter", QVariant::fromValue(senderNameContainsFilter));
}

template <typename T>
QSet<T> toSet(const QList<T> &list) {
    return QSet<T>(list.begin(), list.end());
}

void InboundModel::loadSettings()
{
    QSettings settings;
    settings.beginGroup("inbound");

    senderBlacklist = toSet(settings.value("senderBlacklist").toStringList());
    toBlacklist = toSet(settings.value("toBlacklist").toStringList());
    folderSpecialPurposeBlacklist = settings.value("folderSpecialPurposeBlacklist").toStringList();
    folderNameBlacklist = settings.value("folderNameBlacklist").toStringList();
    senderNameContainsFilter = settings.value("senderNameContainsFilter").toString();

    messageFilter.clear();
    for (const auto &filter : settings.value("messageFilter").toStringList()) {
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

void InboundModel::mailDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &/*roles*/)
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


static QVariantMap toVariantMap(const Sink::ApplicationDomain::Event::Ptr &event, const EventOccurrenceModel::Occurrence &occurrence)
{
    return {
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

        //TODO use identifier + recurrenceid as id here?
        insert(event->identifier(), ::toVariantMap(event, occurrence));
    }
}

void InboundModel::eventRowsRemoved(const QModelIndex &parent, int first, int last)
{
    for (auto row = first; row <= last; row++) {
        auto idx = mEventSourceModel->index(row, 0, parent);

        auto event = idx.data(EventOccurrenceModel::Event).value<Sink::ApplicationDomain::Event::Ptr>();

        for (auto item : mInboundModel->findItems(QString{event->identifier()})) {
            mInboundModel->removeRows(item->row(), 1);
        }
    }
}

void InboundModel::eventDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &/*roles*/)
{
    if (!topLeft.isValid() || !bottomRight.isValid()) {
        return;
    }
    for (auto row = topLeft.row(); row <= bottomRight.row(); row++) {
        auto idx = mEventSourceModel->index(row, 0, topLeft.parent());

        auto event = idx.data(EventOccurrenceModel::Event).value<Sink::ApplicationDomain::Event::Ptr>();
        auto occurrence = idx.data(EventOccurrenceModel::EventOccurrence).value<EventOccurrenceModel::Occurrence>();

        //TODO remove when filtered?

        update(event->identifier(), ::toVariantMap(event, occurrence));
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
    bool initEventModel = !mCurrentDateTime.isValid();
    mCurrentDateTime = dt;
    getAllByType("event", [&](const QModelIndex &index) {
        const auto occurrence = index.data(mRoles["data"]).toMap()["occurrence"].value<EventOccurrenceModel::Occurrence>();
        if (filter(occurrence)) {
            mInboundModel->removeRow(index.row());
        }
        return index.sibling(index.row() + 1, index.column());
    });
    if (initEventModel) {
        refresh();
    }
}


static void requestHeaders(Sink::Query &query)
{
    using namespace Sink::ApplicationDomain;
    query.request<Mail::Subject>();
    query.request<Mail::Sender>();
    query.request<Mail::To>();
    query.request<Mail::Cc>();
    query.request<Mail::Bcc>();
    query.request<Mail::Date>();
    query.request<Mail::Unread>();
    query.request<Mail::Important>();
    query.request<Mail::Draft>();
    query.request<Mail::Folder>();
    query.request<Mail::Sent>();
    query.request<Mail::Trash>();
}

static void requestFullMail(Sink::Query &query)
{
    using namespace Sink::ApplicationDomain;
    requestHeaders(query);
    query.request<Mail::MimeMessage>();
    query.request<Mail::FullPayloadAvailable>();
}

void InboundModel::setFilter(const QVariantMap &filter)
{
    mFilter = filter;
    emit filterChanged();

    if (filter.value("inbound").toBool()) {
        m_model.clear();
        initInboundFilter();
        return;
    }

    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    bool validQuery = false;
    Sink::Query query;

    if (filter.value("folder").value<ApplicationDomainType::Ptr>()) {
        const Folder folder{*filter.value("folder").value<ApplicationDomainType::Ptr>()};
        const auto specialPurpose = folder.getSpecialPurpose();
        bool mIsThreaded = !(specialPurpose.contains(SpecialPurpose::Mail::drafts) ||
                                specialPurpose.contains(SpecialPurpose::Mail::sent));

        query = [&] {
            if (mIsThreaded) {
                return Sink::StandardQueries::threadLeaders(folder);
            } else {
                Sink::Query query;
                query.setId("threadleaders-unthreaded");
                if (!folder.resourceInstanceIdentifier().isEmpty()) {
                    query.resourceFilter(folder.resourceInstanceIdentifier());
                }
                query.filter<Sink::ApplicationDomain::Mail::Folder>(folder);
                query.sort<Sink::ApplicationDomain::Mail::Date>();
                return query;
            }
        }();

        if (!folder.getSpecialPurpose().contains(Sink::ApplicationDomain::SpecialPurpose::Mail::trash)) {
            //Filter trash if this is not a trash folder
            query.filter<Sink::ApplicationDomain::Mail::Trash>(false);
        }

        query.setFlags(Sink::Query::LiveQuery);
        query.limit(100);

        validQuery = true;
    } else if (!filter.value("account").toByteArray().isEmpty()) {
        query.resourceFilter<SinkResource::Account>(filter.value("account").toByteArray());
    }

    if (filter.value("important").toBool()) {
        query.setId("threadLeadersImportant");
        query.setFlags(Sink::Query::LiveQuery);
        query.filter<Sink::ApplicationDomain::Mail::Important>(true);
        query.sort<ApplicationDomain::Mail::Date>();
        query.reduce<ApplicationDomain::Mail::ThreadId>(Query::Reduce::Selector::max<ApplicationDomain::Mail::Date>())
            .count()
            .select<ApplicationDomain::Mail::Subject>(Query::Reduce::Selector::Min)
            .collect<ApplicationDomain::Mail::Unread>()
            .collect<ApplicationDomain::Mail::Important>();
        validQuery = true;
    }

    if (filter.value("recent").toBool()) {
        query.setFlags(Sink::Query::LiveQuery);
        query.filter<Mail::Sent>(true);
        query.filter<Mail::Trash>(false);
        query.sort<ApplicationDomain::Mail::Date>();
        query.reduce<ApplicationDomain::Mail::ThreadId>(Query::Reduce::Selector::max<ApplicationDomain::Mail::Date>())
            .count()
            .select<ApplicationDomain::Mail::Subject>(Query::Reduce::Selector::Min)
            .collect<ApplicationDomain::Mail::Unread>()
            .collect<ApplicationDomain::Mail::Sent>();
        validQuery = true;
    }

    if (filter.value("drafts").toBool()) {
        query.setFlags(Sink::Query::LiveQuery);
        query.filter<Mail::Draft>(true);
        query.filter<Mail::Trash>(false);
        validQuery = true;
    }

    //Additional filtering
    if (filter.value("string").isValid()) {
        validQuery = applyStringFilter(query, filter.value("string").toString());
    }

    if (filter.value("headersOnly").toBool()) {
        requestHeaders(query);
    } else {
        requestFullMail(query);
    }

    if (validQuery) {
        runQuery(query);
    } else {
        setSourceModel(nullptr);
    }
}


void InboundModel::runQuery(const Sink::Query &query)
{
    if (query.getBaseFilters().isEmpty() && query.ids().isEmpty()) {
        m_model.clear();
        setSourceModel(nullptr);
    } else {
        m_model = Sink::Store::loadModel<Sink::ApplicationDomain::Mail>(query);

        QObject::connect(m_model.data(), &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &roles) {
            if (roles.contains(Sink::Store::ChildrenFetchedRole)) {
                emit initialItemsLoaded();
            }
        });
        setSourceModel(m_model.data());
    }
}

QVariantMap InboundModel::filter() const
{
    return {};
}

QHash<int, QByteArray> InboundModel::roleNames() const
{
    return mRoleNames;
}


QVariant InboundModel::data(const QModelIndex &idx, int role) const
{
    // qWarning() << "Getting model data" << idx << role;
    if (m_model) {
        // qWarning() << "Getting model data from model" << idx << role;
        const auto srcIdx = mapToSource(idx);
        const auto mail = srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
        //TODO constexpr to get the role from name and then do a switch?
        const auto roleName = mRoleNames.value(role);
        if (roleName == "type") {
            return "mail";
        }
        if (roleName == "message") {
            return QObject::tr("A new message is available: %1").arg(mail->getSubject());
        }
        if (roleName == "subtype") {
            return "mail";
        }
        if (roleName == "entities") {
            return QVariantList{mail->identifier()};
        }
        if (roleName == "resource") {
            return QString{mail->resourceInstanceIdentifier()};
        }
        if (roleName == "date") {
            return mail->getDate();
        }
        if (roleName == "timestamp") {
            return mail->getDate();
        }
        if (roleName == "data") {
            return QVariantMap{
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
            };
        }
    }
    return QSortFilterProxyModel::data(idx, role);
}
