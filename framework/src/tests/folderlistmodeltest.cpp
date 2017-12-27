#include <QTest>
#include <QDebug>
#include <QStandardItemModel>
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
};

QTEST_MAIN(FolderlistModelTest)
#include "folderlistmodeltest.moc"
