#pragma once

#include "maillistmodel.h"

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QAbstractItemModel>

class MailListController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString folderId READ folderId WRITE setFolderId NOTIFY folderIdChanged)
    Q_PROPERTY (MailListModel *model READ model CONSTANT)

public:
    explicit MailListController(QObject *parent = Q_NULLPTR);

    QString folderId() const;
    void setFolderId(const QString &query);
    MailListModel *model() const;

signals:
    void folderIdChanged();

public slots:
    void addMail(QString subject);

private:
    QString m_folderId;
    QScopedPointer<MailListModel> m_model;
};
