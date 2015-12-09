#include "maillistcontroller.h"

#include <QStringList>

#include <akonadi2common/clientapi.h>

#include "maillistmodel.h"

MailListController::MailListController(QObject *parent) : QObject(parent), m_model(new MailListModel)
{
}

MailListModel *MailListController::model() const
{
    return m_model.data();

}

QString MailListController::folderId() const
{
    return m_folderId;
}

void MailListController::setFolderId(const QString &folderId)
{
    if (m_folderId != folderId) {
        m_folderId = folderId;


        Akonadi2::Query query;
        query.syncOnDemand = false;
        query.processAll = false;
        query.liveQuery = true;
        query.requestedProperties << "subject" << "sender" << "senderName" << "date" << "unread" << "important" << "folder";
        query.propertyFilter.insert("folder", folderId.toLatin1());
        m_model->runQuery(query);

        emit folderIdChanged();
    }
}

void MailListController::addMail(QString subject)
{
    qDebug() << "add mail";
}
