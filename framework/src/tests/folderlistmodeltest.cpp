#include <QTest>
#include <QDebug>
#include <QStandardItemModel>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include "krecursivefilterproxymodel.h"
#include "folderlistmodel.h"

class TestModel : public KRecursiveFilterProxyModel {
public:

    TestModel()
        :KRecursiveFilterProxyModel()
    {
        auto model = QSharedPointer<QStandardItemModel>::create();
        setSourceModel(model.data());
        mModel = model;
    }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE
    {
        return left.data(Qt::DisplayRole).toString() < right.data(Qt::DisplayRole).toString();
    }

    bool acceptRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE
    {
        auto index = sourceModel()->index(sourceRow, 0, sourceParent);
        return index.data(Qt::DisplayRole).toString().contains("accept");
    }

    QSharedPointer<QStandardItemModel> mModel;
};

bool contains(QAbstractItemModel &model, const QModelIndex &parent, QString s)
{
    for (int row = 0; row < model.rowCount(parent); row++) {
        auto idx = model.index(row, 0, parent);
        if (idx.data(Qt::DisplayRole).toString() == s) {
            return true;
        }
        if (contains(model, idx, s)) {
            return true;
        }
    }
    return false;
}

class FolderlistModelTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testRecursiveFilterModel()
    {
        TestModel model;
        auto root = new QStandardItem{"acceptroot"};
        model.mModel->appendRow(root);
        auto item = new QStandardItem{"accept1"};
        root->appendRow(item);
        item->appendRow(new QStandardItem{"accept11"});
        QVERIFY(contains(model, {}, "acceptroot"));
        QVERIFY(contains(model, {}, "accept1"));
        QVERIFY(contains(model, {}, "accept11"));
    }

    void testFolderListModel()
    {
        Sink::ApplicationDomain::DummyResource::create("account1");

        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();

        auto folder = ApplicationDomainType::createEntity<Folder>(resource.identifier());
        Sink::Store::create(folder).exec().waitForFinished();

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        FolderListModel model;
        model.setAccountId(account.identifier());
        QTRY_COMPARE(model.rowCount({}), 1);
        {
            auto subfolder = ApplicationDomainType::createEntity<Folder>(resource.identifier());
            subfolder.setParent(folder);
            Sink::Store::create(subfolder).exec().waitForFinished();
            Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();
        }
        QTRY_COMPARE(model.rowCount(model.index(0, 0, {})), 1);
    }
};

QTEST_MAIN(FolderlistModelTest)
#include "folderlistmodeltest.moc"
