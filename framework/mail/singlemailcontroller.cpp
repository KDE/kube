#include "singlemailcontroller.h"

SingleMailController::SingleMailController(QObject *parent) : QObject(parent), m_model(new MailListModel)
{

}

SingleMailController::~SingleMailController()
{

}

MailListModel* SingleMailController::model() const
{
    return m_model.data();
}


void SingleMailController::loadMail(const QString &id)
{
    Akonadi2::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = false;
    query.requestedProperties << "subject" << "sender" << "senderName" << "date" << "unread" << "important" << "mimeMessage";
    query.ids << id.toLatin1();
    m_model->runQuery(query);
}