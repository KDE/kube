#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QAbstractItemModel>

class FolderListController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString accountId READ accountId WRITE setAccountId NOTIFY accountIdChanged)
    Q_PROPERTY (QAbstractItemModel *model READ model CONSTANT)

public:
    explicit FolderListController(QObject *parent = Q_NULLPTR);

    QString accountId() const;
    void setAccountId(const QString &id);

    QAbstractItemModel *model() const;

    void loadFolders(const QString &id);

signals:
    void accountIdChanged();

public slots:
    void deleteFolder(const QString &id);
    void addFolder(const QString &name);


private:
    QString m_accountId;
    QScopedPointer<QAbstractItemModel> m_model;
};
