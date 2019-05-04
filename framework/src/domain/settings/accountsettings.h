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

#include "kube_export.h"
#include <QObject>
#include <QValidator>

class KUBE_EXPORT AccountSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray accountType READ accountType WRITE setAccountType)
    Q_PROPERTY(QByteArray accountIdentifier READ accountIdentifier WRITE setAccountIdentifier NOTIFY changed)
    Q_PROPERTY(QString icon MEMBER mIcon NOTIFY changed)
    Q_PROPERTY(QString accountName MEMBER mName NOTIFY changed)

    Q_PROPERTY(QString userName MEMBER mUsername NOTIFY identityChanged)
    Q_PROPERTY(QString emailAddress MEMBER mEmailAddress NOTIFY identityChanged)

    Q_PROPERTY(QString imapServer MEMBER mImapServer NOTIFY imapResourceChanged)
    Q_PROPERTY(QValidator* imapServerValidator READ imapServerValidator CONSTANT)
    Q_PROPERTY(QString imapUsername MEMBER mImapUsername NOTIFY imapResourceChanged)

    Q_PROPERTY(QString smtpServer MEMBER mSmtpServer NOTIFY smtpResourceChanged)
    Q_PROPERTY(QValidator* smtpServerValidator READ smtpServerValidator CONSTANT)
    Q_PROPERTY(QString smtpUsername MEMBER mSmtpUsername NOTIFY smtpResourceChanged)

    Q_PROPERTY(QString carddavServer MEMBER mCardDavServer NOTIFY cardDavResourceChanged)
    Q_PROPERTY(QString carddavUsername MEMBER mCardDavUsername NOTIFY cardDavResourceChanged)

    Q_PROPERTY(QString caldavServer MEMBER mCalDavServer NOTIFY calDavResourceChanged)
    Q_PROPERTY(QString caldavUsername MEMBER mCalDavUsername NOTIFY calDavResourceChanged)

    Q_PROPERTY(QUrl path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QValidator* pathValidator READ pathValidator CONSTANT)

public:
    AccountSettings(QObject *parent = 0);

    void setAccountIdentifier(const QByteArray &);
    QByteArray accountIdentifier() const;

    void setAccountType(const QByteArray &);
    QByteArray accountType() const;

    void setPath(const QUrl &);
    QUrl path() const;

    virtual QValidator *imapServerValidator() const;
    virtual QValidator *smtpServerValidator() const;
    virtual QValidator *pathValidator() const;

    Q_INVOKABLE virtual void load();
    Q_INVOKABLE virtual void save();
    Q_INVOKABLE virtual void remove();
    Q_INVOKABLE void login(const QVariantMap &secrets);

signals:
    void imapResourceChanged();
    void smtpResourceChanged();
    void identityChanged();
    void pathChanged();
    void changed();
    void cardDavResourceChanged();
    void calDavResourceChanged();

protected:
    void saveAccount();
    void saveImapResource();
    void saveMaildirResource();
    void saveMailtransportResource();
    void saveIdentity();
    void saveCardDavResource();
    void saveCalDavResource();

    void loadAccount();
    void loadImapResource();
    void loadMaildirResource();
    void loadMailtransportResource();
    void loadIdentity();
    void loadCardDavResource();
    void loadCalDavResource();

    void removeAccount();
    void removeResource(const QByteArray &identifier);

    void removeIdentity();

    QByteArray mAccountIdentifier;
    QByteArray mAccountType;
    QString mIcon;
    QString mName;

    QByteArray mImapIdentifier;
    QString mImapServer;
    QString mImapUsername;

    QByteArray mMaildirIdentifier;
    QString mPath;

    QByteArray mMailtransportIdentifier;
    QString mSmtpServer;
    QString mSmtpUsername;

    QByteArray mIdentityIdentifier;
    QString mUsername;
    QString mEmailAddress;

    QByteArray mCardDavIdentifier;
    QString mCardDavServer;
    QString mCardDavUsername;

    QByteArray mCalDavIdentifier;
    QString mCalDavServer;
    QString mCalDavUsername;
};

