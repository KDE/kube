#pragma once

#include "folderlistmodel.h"

#include <QObject>
#include <QScopedPointer>
#include <QString>

class FolderListController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString accountId READ accountId WRITE setAccountId NOTIFY accountIdChanged)
    Q_PROPERTY (FolderListModel *model READ model CONSTANT)

public:
    explicit FolderListController(QObject *parent = Q_NULLPTR);

    QString accountId() const;
    void setAccountId(const QString &id);

    FolderListModel *model() const;

    void loadFolders(const QString &id);

signals:
    void accountIdChanged();

public slots:
    void deleteFolder(const QString &id);
    void addFolder(const QString &name);


private:
    QString m_accountId;
    QScopedPointer<FolderListModel> m_model;
};
