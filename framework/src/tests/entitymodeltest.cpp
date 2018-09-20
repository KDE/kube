#include <QTest>
#include <QDebug>
#include <QStandardItemModel>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>

#include "entitymodel.h"

class EntityModelTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testModel()
    {
        Sink::ApplicationDomain::DummyResource::create("account1");

        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();

        auto calendar1 = ApplicationDomainType::createEntity<Calendar>(resource.identifier());
        calendar1.setName("name1");
        Sink::Store::create(calendar1).exec().waitForFinished();

        auto calendar2 = ApplicationDomainType::createEntity<Calendar>(resource.identifier());
        calendar2.setName("name2");
        Sink::Store::create(calendar2).exec().waitForFinished();


        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            EntityModel model;
            model.setType("calendar");
            model.setRoles({"name"});
            model.setAccountId(account.identifier());
            QTRY_COMPARE(model.rowCount({}), 2);
        }

        {
            EntityModel model;
            model.setType("calendar");
            model.setRoles({"name"});
            model.setAccountId(account.identifier());
            model.setEntityId(calendar2.identifier());
            QTRY_COMPARE(model.rowCount({}), 1);
            QCOMPARE(model.data(0).value("name").toString(), {"name2"});
        }

        {
            EntityLoader model;
            model.setType("calendar");
            model.setRoles({"name"});
            model.setAccountId(account.identifier());
            model.setEntityId(calendar2.identifier());
            QTRY_COMPARE(model.rowCount({}), 1);
            QCOMPARE(model.property("name").toString(), {"name2"});
        }
    }
};

QTEST_MAIN(EntityModelTest)
#include "entitymodeltest.moc"
