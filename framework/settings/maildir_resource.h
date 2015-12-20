#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class MaildirResouceController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY (QUrl fodlerUrl READ folderUrl WRITE setFolderUrl NOTIFY folderUrlChanged);

public:
    explicit MaildirResouceController(QObject *parent = Q_NULLPTR);

    QString name() const;
    void setName(const QString &name);

    QUrl folderUrl() const;
    void setFolderUrl(const QUrl &url);

signals:
    void nameChanged();
    void folderUrlChanged();

private:
    QString m_name;
    QUrl m_folderUrl;
};