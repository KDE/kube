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
    query.requestedProperties << "subject" << "sender" << "senderName" << "date" << "unread" << "important";
    query.ids << id.toLatin1();
    m_model->runQuery(query);

    qDebug() << "***";
    auto srcIdx = m_model->mapToSource(m_model->index(1, 0));
    auto bla = srcIdx.sibling(srcIdx.row(), 3).data(Qt::DisplayRole);
    qDebug() << bla;
    qDebug() << "***";
}