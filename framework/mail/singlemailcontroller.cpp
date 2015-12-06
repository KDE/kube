#include "singlemailcontroller.h"

#include <QDebug>

SingleMailController::SingleMailController(QObject *parent): QObject(parent), m_isImportant(false), m_isUnread(true)
{

}

QString SingleMailController::akonadiId() const
{
    return m_akonadiId;
}

void SingleMailController::setAkonadiId(const QString &id)
{
    if(m_akonadiId != id) {
        m_akonadiId = id;

	loadMessage(m_akonadiId);

        emit akonadiIdChanged();
    }
}

QString SingleMailController::message() const
{
    return m_message;
}

bool SingleMailController::isImportant() const
{
    return m_isImportant;
}

bool SingleMailController::isUnread() const
{
    return m_isUnread;
}

void SingleMailController::deleteMail()
{
    qDebug() << "UserAction: deleteMail: " << m_akonadiId;
}

void SingleMailController::markMailImportant(bool important)
{
    qDebug() << "UserAction: markMailImportant: " << m_akonadiId;

    if (m_isImportant != important) {
        m_isImportant = important;
        emit isImportantChanged();
    }
}

void SingleMailController::markMailUnread(bool unread)
{
    qDebug() << "UserAction: markMailUnread: " << m_akonadiId;

    if (m_isUnread != unread) {
        m_isUnread = unread;
        emit isUnreadChanged();
    }
}

void SingleMailController::loadMessage(const QString &id)
{
    //load message from akoandi
    m_message = "The message as HTML";
    emit messageChanged();
}
