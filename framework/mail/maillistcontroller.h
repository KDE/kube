#pragma once

#include "maillistmodel.h"

#include <QObject>
#include <QScopedPointer>
#include <QString>

class MailListController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY (MailListModel *model READ model CONSTANT)

public:
    explicit MailListController(QObject *parent = Q_NULLPTR);

    QString query() const;
    void setQuery(const QString &query);
    MailListModel *model() const;

signals:
    void queryChanged();

public slots:
    void addMail(QString subject);

private:
    QString m_query;
    QScopedPointer<MailListModel> m_model;
};