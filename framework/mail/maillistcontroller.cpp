#include "maillistcontroller.h"

#include <QStringList>

#include <akonadi2common/clientapi.h>
#include <akonadi2common/query.h>

#include "maillistmodel.h"

MailListController::MailListController(QObject *parent) : QObject(parent), m_model(new MailListModel)
{
}

QAbstractItemModel *MailListController::model() const
{
    return m_model.data();

}

QString MailListController::query() const
{
    return m_query;
}

void MailListController::setQuery(const QString &query)
{
    qDebug() << "set query";
    if (m_query != query) {
        m_query = query;
        emit queryChanged();
    }
}

void MailListController::addMail(QString subject)
{
    qDebug() << "add mail";
}
