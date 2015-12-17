#pragma once

#include "maillistmodel.h"

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QScopedPointer>

class SingleMailController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (MailListModel *model READ model CONSTANT)

public:
    explicit SingleMailController(QObject *parent = Q_NULLPTR);
    ~SingleMailController();

    MailListModel *model() const;

Q_SIGNALS:
    void messageChanged();

public slots:
    void loadMail(const QString &id);

private:
    QScopedPointer<MailListModel> m_model;
};
