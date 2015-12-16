#pragma once

#include "maillistmodel.h"

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QAbstractItemModel>

class MailListController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (MailListModel *model READ model CONSTANT)
    Q_PROPERTY (QString selectedMail READ selectedMail WRITE setSelectedMail NOTIFY selectedMailChanged)

public:
    explicit MailListController(QObject *parent = Q_NULLPTR);

    MailListModel *model() const;

    QString selectedMail() const;
    void setSelectedMail(const QString &id);

signals:
    void selectedMailChanged();

public slots:
    void loadAllMail();
    void loadUnreadMail();
    void loadImportantMail();
    void loadMailFolder(const QString &folderId);

    void markMailImportant(bool important);
    void markMailUnread(bool unread);
    void deleteMail();

private:
    QScopedPointer<MailListModel> m_model;

    QString m_selectedMail;
};
