/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QAbstractItemModel>

namespace KMime {
class Message;
}

class ComposerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QVariant originalMessage WRITE setOriginalMessage)
    Q_PROPERTY (QVariant draftMessage WRITE setDraftMessage)
    Q_PROPERTY (QString to READ to WRITE setTo NOTIFY toChanged)
    Q_PROPERTY (QString cc READ cc WRITE setCc NOTIFY ccChanged)
    Q_PROPERTY (QString bcc READ bcc WRITE setBcc NOTIFY bccChanged)
    Q_PROPERTY (QString subject READ subject WRITE setSubject NOTIFY subjectChanged)
    Q_PROPERTY (QString body READ body WRITE setBody NOTIFY bodyChanged)
    Q_PROPERTY (QAbstractItemModel* identityModel READ identityModel CONSTANT)
    Q_PROPERTY (int currentIdentityIndex MEMBER m_currentAccountIndex)
    Q_PROPERTY (QStringList attachments READ attachemts NOTIFY attachmentsChanged)

public:
    explicit ComposerController(QObject *parent = Q_NULLPTR);

    QString to() const;
    void setTo(const QString &to);

    QString cc() const;
    void setCc(const QString &cc);

    QString bcc() const;
    void setBcc(const QString &bcc);

    QString subject() const;
    void setSubject(const QString &subject);

    QString body() const;
    void setBody(const QString &body);

    QAbstractItemModel *identityModel() const;

    QStringList attachemts() const;

    void setOriginalMessage(const QVariant &originalMessage);
    void setDraftMessage(const QVariant &draft);

signals:
    void subjectChanged();
    void bodyChanged();
    void toChanged();
    void ccChanged();
    void bccChanged();
    void fromIndexChanged();
    void attachmentsChanged();

public slots:
    void send();
    void saveAsDraft();
    void clear();
    void addAttachment(const QUrl &fileUrl);

private:
    void setMessage(const QSharedPointer<KMime::Message> &msg);
    QSharedPointer<KMime::Message> assembleMessage();
    QString m_to;
    QString m_cc;
    QString m_bcc;
    QString m_subject;
    QString m_body;
    QStringList m_attachments;
    QVariant m_msg;
    int m_currentAccountIndex;
};
