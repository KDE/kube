#include "maillistcontroller.h"

#include <QStringList>

MailListController::MailListController(QObject *parent) : QObject(parent), m_model(new MailListModel)
{
}

MailListModel *MailListController::model() const
{
    return m_model.data();
}

QString MailListController::query() const
{
    return m_query;
}

void MailListController::setQuery(const QString &query)
{
    if ( m_query != query) {
	m_query = query;
	m_model->runQuery(query);
	emit queryChanged();
    }
}

void MailListController::addMail(QString subject)
{
  QStringList list;

  list << subject;

  m_model->addMails(list);
}
