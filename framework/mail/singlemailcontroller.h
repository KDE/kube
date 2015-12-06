#pragma once

#include "maillistmodel.h"

#include <QObject>
#include <QString>

class SingleMailController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString akonadiId READ akonadiId WRITE setAkonadiId NOTIFY akonadiIdChanged)
    Q_PROPERTY (bool isUnread READ isUnread NOTIFY isUnreadChanged)
    Q_PROPERTY (bool isImportant READ isImportant NOTIFY isImportantChanged)
    Q_PROPERTY (QString message READ message NOTIFY messageChanged)

public:
    explicit SingleMailController(QObject *parent = Q_NULLPTR);

    QString akonadiId() const;
    void setAkonadiId(const QString &id);

    bool isUnread() const;
    bool isImportant() const;
    QString message() const;

    void loadMessage(const QString &id);

signals:
    void akonadiIdChanged();
    void isUnreadChanged();
    void isImportantChanged();
    void messageChanged();

public slots:
    void deleteMail();
    void markMailImportant(bool important);
    void markMailUnread(bool unread);

private:
    QString m_akonadiId;
    bool m_isImportant;
    bool m_isUnread;
    QString m_message;
};
