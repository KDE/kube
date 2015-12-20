#include "maildir_resource.h"

MaildirResouceController::MaildirResouceController(QObject *parent) : QObject(parent)
{

}

QString MaildirResouceController::name() const
{
    return m_name;
}

void MaildirResouceController::setName(const QString &name)
{
    if(m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

QUrl MaildirResouceController::folderUrl() const
{
    return m_folderUrl;
}

void MaildirResouceController::setFolderUrl(const QUrl &url)
{
    if(m_folderUrl != url) {
        m_folderUrl = url;
        emit folderUrlChanged();
    }
}
