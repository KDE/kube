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

public:
    explicit MailListController(QObject *parent = Q_NULLPTR);

    MailListModel *model() const;

public slots:
    void loadAllMail();
    void loadUnreadMail();
    void loadImportantMail();
    void loadMailFolder(const QString &folderId);

private:
    QScopedPointer<MailListModel> m_model;
};
