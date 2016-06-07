/*
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include <QValidator>

class ImapSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray accountIdentifier READ accountIdentifier WRITE setAccountIdentifier)
    Q_PROPERTY(QString icon MEMBER mIcon NOTIFY changed)
    Q_PROPERTY(QString accountName MEMBER mName NOTIFY changed)
    Q_PROPERTY(QString userName MEMBER mUsername NOTIFY identityChanged)
    Q_PROPERTY(QString emailAddress MEMBER mEmailAddress NOTIFY identityChanged)
    Q_PROPERTY(QString imapServer MEMBER mImapServer NOTIFY imapResourceChanged)
    Q_PROPERTY(QValidator* imapServerValidator READ imapServerValidator CONSTANT)
    Q_PROPERTY(QString imapUsername MEMBER mImapUsername NOTIFY imapResourceChanged)
    Q_PROPERTY(QString imapPassword MEMBER mImapPassword NOTIFY imapResourceChanged)
    Q_PROPERTY(QString smtpServer MEMBER mSmtpServer NOTIFY smtpResourceChanged)
    Q_PROPERTY(QValidator* smtpServerValidator READ smtpServerValidator CONSTANT)
    Q_PROPERTY(QString smtpUsername MEMBER mSmtpUsername NOTIFY smtpResourceChanged)
    Q_PROPERTY(QString smtpPassword MEMBER mSmtpPassword NOTIFY smtpResourceChanged)

public:
    ImapSettings(QObject *parent = 0);

    void setAccountIdentifier(const QByteArray &);
    QByteArray accountIdentifier() const;

    QValidator *imapServerValidator() const;
    QValidator *smtpServerValidator() const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void remove();

signals:
    void imapResourceChanged();
    void smtpResourceChanged();
    void identityChanged();
    void changed();

private:
    QByteArray mIdentifier;
    QByteArray mAccountIdentifier;
    QByteArray mMailtransportIdentifier;
    QByteArray mIdentityIdentifier;
    QString mIcon;
    QString mName;
    QString mUsername;
    QString mEmailAddress;
    QString mImapServer;
    QString mImapUsername;
    QString mImapPassword;
    QString mSmtpServer;
    QString mSmtpUsername;
    QString mSmtpPassword;
};
