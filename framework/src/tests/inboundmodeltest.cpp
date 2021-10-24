
#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include <KCalCore/Event>
#include <KCalCore/ICalFormat>
#include "mailtemplates.h"

#include "inboundmodel.h"

class InboundModelTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testInboundModel()
    {
        Sink::ApplicationDomain::DummyResource::create("account1");

        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        VERIFYEXEC(Sink::Store::create(account));

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        VERIFYEXEC(Sink::Store::create(resource));

        auto calendar1 = ApplicationDomainType::createEntity<Calendar>(resource.identifier());
        calendar1.setEnabled(true);
        VERIFYEXEC(Sink::Store::create(calendar1));

        const QDateTime start{{2018, 04, 17}, {6, 0, 0}};
        {
            auto event1 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event1");
            calcoreEvent->setSummary("summary1");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start);
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event1.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event1.setCalendar(calendar1);
            VERIFYEXEC(Sink::Store::create(event1));
        }
        {
            auto event2 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event2");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            calcoreEvent->recurrence()->setDaily(2);
            event2.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event2.setCalendar(calendar1);
            VERIFYEXEC(Sink::Store::create(event2));
        }

        VERIFYEXEC(Sink::ResourceControl::flushMessageQueue(resource.identifier()));

        InboundModel model;

        {
            QSignalSpy initialItemsLoadedSpy(&model, &InboundModel::initialItemsLoaded);
            model.setCurrentDate(start);
            model.configure(
                {}, // QSet<QString> &_senderBlacklist,
                {}, // QSet<QString> &_toBlacklist,
                {}, // QString &_senderNameContainsFilter,
                {}, // QMap<QString, QString> &_perFolderMimeMessageWhitelistFilter,
                {}, // QList<QRegularExpression> &_messageFilter,
                {}, // QList<QString> &_folderSpecialPurposeBlacklist,
                {}  // QList<QString> &_folderNameBlacklist,
            );

            QTest::qWait(200);
            QTRY_COMPARE(model.rowCount({}), 4);
            QCOMPARE(initialItemsLoadedSpy.count(), 1);
        }

        {

            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event1");
            calcoreEvent->setSummary("summary1");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start);
            calcoreEvent->setDuration(7200);
            calcoreEvent->setAllDay(false);

            const auto event = model.index(3, 0, {}).data(model.roleNames().key("data")).toMap()["domainObject"].value<Sink::ApplicationDomain::Event::Ptr>();
            event->setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());

            QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
            QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
            QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
            QSignalSpy dataChangedSpy(&model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));
            QSignalSpy initialItemsLoadedSpy(&model, &InboundModel::initialItemsLoaded);

            VERIFYEXEC(Sink::Store::modify(*event));
            VERIFYEXEC(Sink::ResourceControl::flushMessageQueue(resource.identifier()));

            QTest::qWait(200);
            QCOMPARE(resetSpy.count(), 0);
            QCOMPARE(rowsRemovedSpy.count(), 0);
            QCOMPARE(rowsInsertedSpy.count(), 0);
            QCOMPARE(dataChangedSpy.count(), 2);
            QCOMPARE(initialItemsLoadedSpy.count(), 0);
        }


        //Filter by date
        {
            QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
            QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
            QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
            model.setCurrentDate(start.addSecs(3600));

            QTest::qWait(200);
            QCOMPARE(resetSpy.count(), 0);
            QCOMPARE(rowsRemovedSpy.count(), 1);
            QCOMPARE(rowsInsertedSpy.count(), 0);
        }
        {
            QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
            QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
            QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
            model.setCurrentDate(start.addSecs(3600));

            QTest::qWait(200);
            QCOMPARE(resetSpy.count(), 0);
            QCOMPARE(rowsRemovedSpy.count(), 0);
            QCOMPARE(rowsInsertedSpy.count(), 0);
        }
    }

    void testMailInbound()
    {
        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        VERIFYEXEC(Sink::Store::create(account));

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        VERIFYEXEC(Sink::Store::create(resource));

        auto folder1 = ApplicationDomainType::createEntity<Folder>(resource.identifier());
        VERIFYEXEC(Sink::Store::create(folder1));

        auto folder2 = ApplicationDomainType::createEntity<Folder>(resource.identifier());
        VERIFYEXEC(Sink::Store::create(folder2));


        auto mail1 = ApplicationDomainType::createEntity<Mail>(resource.identifier());
        mail1.setFolder(folder1);
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        auto message = MailTemplates::createMessage({}, {"foo@test.com"}, {}, {}, from, "Subject", "Body", false, {}, {}, {});
        mail1.setMimeMessage(message->encodedContent(true));
        VERIFYEXEC(Sink::Store::create(mail1));

        auto mail2 = ApplicationDomainType::createEntity<Mail>(resource.identifier());
        mail2.setFolder(folder2);
        VERIFYEXEC(Sink::Store::create(mail2));

        VERIFYEXEC(Sink::ResourceControl::flushMessageQueue(resource.identifier()));


        InboundModel model;
        QSignalSpy initialItemsLoadedSpy(&model, &InboundModel::initialItemsLoaded);
        model.setCurrentDate({});
        model.configure(
            {}, // QSet<QString> &_senderBlacklist,
            {}, // QSet<QString> &_toBlacklist,
            {}, // QString &_senderNameContainsFilter,
            {}, // QMap<QString, QString> &_perFolderMimeMessageWhitelistFilter,
            {}, // QList<QRegularExpression> &_messageFilter,
            {}, // QList<QString> &_folderSpecialPurposeBlacklist,
            {}  // QList<QString> &_folderNameBlacklist,
        );

        //FIXME
        // QTRY_COMPARE(initialItemsLoadedSpy.count(), 1);
        QTRY_COMPARE(model.rowCount({}), 2);

        //Test move to trash
        {
            QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
            QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
            QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
            QSignalSpy dataChangedSpy(&model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));
            QSignalSpy initialItemsLoadedSpy(&model, &InboundModel::initialItemsLoaded);

            auto idx = model.index(0, 0, {});
            auto mail = idx.data(model.roleNames().key("data")).toMap().value("mail").value<Mail::Ptr>();
            mail->setTrash(true);

            VERIFYEXEC(Sink::Store::modify(*mail));
            VERIFYEXEC(Sink::ResourceControl::flushMessageQueue(resource.identifier()));

            QTRY_COMPARE(rowsRemovedSpy.count(), 1);
            QCOMPARE(resetSpy.count(), 0);
            QCOMPARE(rowsInsertedSpy.count(), 0);
            QCOMPARE(dataChangedSpy.count(), 0);

            QTRY_COMPARE(model.rowCount({}), 1);
        }
    }

    void testMailFolder()
    {
        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        VERIFYEXEC(Sink::Store::create(account));

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        VERIFYEXEC(Sink::Store::create(resource));

        auto folder1 = ApplicationDomainType::createEntity<Folder>(resource.identifier());
        VERIFYEXEC(Sink::Store::create(folder1));

        auto folder2 = ApplicationDomainType::createEntity<Folder>(resource.identifier());
        VERIFYEXEC(Sink::Store::create(folder2));

        auto mail1 = ApplicationDomainType::createEntity<Mail>(resource.identifier());
        mail1.setFolder(folder1);
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        auto message = MailTemplates::createMessage({}, {"foo@test.com"}, {}, {}, from, "Subject", "Body", false, {}, {}, {});
        mail1.setMimeMessage(message->encodedContent(true));
        VERIFYEXEC(Sink::Store::create(mail1));

        auto mail2 = ApplicationDomainType::createEntity<Mail>(resource.identifier());
        mail2.setFolder(folder2);
        VERIFYEXEC(Sink::Store::create(mail2));

        VERIFYEXEC(Sink::ResourceControl::flushMessageQueue(resource.identifier()));


        InboundModel model;
        QSignalSpy initialItemsLoadedSpy(&model, &InboundModel::initialItemsLoaded);
        model.setFilter({{"folder", QVariant::fromValue(ApplicationDomainType::Ptr::create(folder1))}});

        QTRY_COMPARE(initialItemsLoadedSpy.count(), 1);
        QCOMPARE(model.rowCount({}), 1);
        {
            auto idx = model.index(0, 0, {});
            auto mail = idx.data(model.roleNames().key("data")).toMap().value("mail").value<Mail::Ptr>();
            QVERIFY(mail);
            QVERIFY(!mail->getSubject().isEmpty());
        }

        //Test move to trash
        {
            QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
            QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
            QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
            QSignalSpy dataChangedSpy(&model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));
            QSignalSpy initialItemsLoadedSpy(&model, &InboundModel::initialItemsLoaded);

            auto idx = model.index(0, 0, {});
            auto mail = idx.data(model.roleNames().key("data")).toMap().value("mail").value<Mail::Ptr>();
            mail->setTrash(true);

            VERIFYEXEC(Sink::Store::modify(*mail));
            VERIFYEXEC(Sink::ResourceControl::flushMessageQueue(resource.identifier()));

            QTRY_COMPARE(rowsRemovedSpy.count(), 1);
            QCOMPARE(resetSpy.count(), 0);
            QCOMPARE(rowsInsertedSpy.count(), 0);
            QCOMPARE(dataChangedSpy.count(), 0);

            QTRY_COMPARE(model.rowCount({}), 0);
        }
    }

};

QTEST_MAIN(InboundModelTest)
#include "inboundmodeltest.moc"
