#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class MaildirResouceController : public QObject
{
    Q_OBJECT

public:
    explicit MaildirResouceController(QObject *parent = Q_NULLPTR);


private:
    QString m_name;
    QUrl m_folderLocation;
};