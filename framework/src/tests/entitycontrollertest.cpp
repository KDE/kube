#include <QTest>
#include <QDebug>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>

#include "entitycontroller.h"
#include "entitymodel.h"

class EntityControllerTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testCreate()
    {
        Sink::ApplicationDomain::DummyResource::create("account1");

        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();

        EntityController controller;
        controller.create({
                {"type", "calendar"},
                {"account", account.identifier()},
                {"entity", QVariantMap {
                        {"name", "name2"}
                    }
                }
                });


        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            const auto calendars = Sink::Store::read<Sink::ApplicationDomain::Calendar>({});
            QCOMPARE(calendars.size(), 1);
            QCOMPARE(calendars.first().getName(), QLatin1String{"name2"});
        }
    }
};

QTEST_MAIN(EntityControllerTest)
#include "entitycontrollertest.moc"
