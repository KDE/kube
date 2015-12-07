#include "folderlistcontroller.h"

#include "folderlistmodel.h"

#include <QDebug>

FolderListController::FolderListController(QObject *parent) : QObject(parent), m_model(new FolderListModel)
{

}

QString FolderListController::accountId() const
{
    return m_accountId;
}

void FolderListController::setAccountId(const QString &id)
{
    if(m_accountId != id) {
        m_accountId = id;

	loadFolders(id);

	emit accountIdChanged();
    }
}

FolderListModel* FolderListController::model() const
{
    return m_model.data();
}

void FolderListController::loadFolders(const QString &id)
{
    //load foldermodel from akonadi

}


void FolderListController::addFolder(const QString &name)
{
    qDebug() << "User Action: add folder " << name;
}

void FolderListController::deleteFolder(const QString &id)
{
    qDebug() << "User Action: delete folder " << id;
}

