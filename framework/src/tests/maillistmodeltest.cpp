#include <QTest>
#include <QDebug>
#include <QStandardItemModel>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include "maillistmodel.h"
#include "mailtemplates.h"

class MaillistModelTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testMailListModel()
    {
        Sink::ApplicationDomain::DummyResource::create("account1");

        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();

        auto folder1 = ApplicationDomainType::createEntity<Folder>(resource.identifier());
        Sink::Store::create(folder1).exec().waitForFinished();

        auto folder2 = ApplicationDomainType::createEntity<Folder>(resource.identifier());
        Sink::Store::create(folder2).exec().waitForFinished();


        auto mail1 = ApplicationDomainType::createEntity<Mail>(resource.identifier());
        mail1.setFolder(folder1);
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        auto message = MailTemplates::createMessage({}, {"foo@test.com"}, {}, {}, from, "Subject", "Body", false, {}, {}, {});
        mail1.setMimeMessage(message->encodedContent(true));
        Sink::Store::create(mail1).exec().waitForFinished();

        auto mail2 = ApplicationDomainType::createEntity<Mail>(resource.identifier());
        mail2.setFolder(folder2);
        Sink::Store::create(mail2).exec().waitForFinished();

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            MailListModel model;
            model.setParentFolder(QVariant::fromValue(Folder::Ptr::create(folder1)));
            QTRY_COMPARE(model.rowCount({}), 1);

            {
                auto idx = model.index(0, 0, {});
                auto mail = idx.data(MailListModel::DomainObject).value<Mail::Ptr>();
                QVERIFY(mail);
                QVERIFY(!mail->getSubject().isEmpty());
            }
        }
        {
            MailListModel model;
            model.setMail(QVariant::fromValue(Mail::Ptr::create(mail1)));
            QTRY_COMPARE(model.rowCount({}), 1);

            {
                auto idx = model.index(0, 0, {});
                auto mail = idx.data(MailListModel::DomainObject).value<Mail::Ptr>();
                QVERIFY(mail);
                QVERIFY(!mail->getSubject().isEmpty());
                QVERIFY(mail->getFullPayloadAvailable());
            }
        }
    }
};

QTEST_MAIN(MaillistModelTest)
#include "maillistmodeltest.moc"
