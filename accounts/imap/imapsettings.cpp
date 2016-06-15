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
#include "imapsettings.h"

#include <settings/settings.h>

#include <sink/store.h>
#include <QDebug>
#include <QDir>
#include <QUrl>

ImapSettings::ImapSettings(QObject *parent)
    : QObject(parent)
{
}


void ImapSettings::setAccountIdentifier(const QByteArray &id)
{
    if (id.isEmpty()) {
        return;
    }
    mAccountIdentifier = id;

    //Clear
    mIcon = QString();
    mName = QString();
    mImapServer = QString();
    mImapUsername = QString();
    mImapPassword = QString();
    mSmtpServer = QString();
    mSmtpUsername = QString();
    mSmtpPassword = QString();
    emit changed();
    emit imapResourceChanged();
    emit smtpResourceChanged();

    Q_ASSERT(!id.isEmpty());
    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkAccount>(Sink::Query::IdentityFilter(id))
        .then<void, Sink::ApplicationDomain::SinkAccount>([this](const Sink::ApplicationDomain::SinkAccount &account) {
            mIcon = account.getProperty("icon").toString();
            mName = account.getProperty("name").toString();
            emit changed();
        }).exec();

    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkResource>(Sink::Query::AccountFilter(id) + Sink::Query::CapabilityFilter(Sink::ApplicationDomain::ResourceCapabilities::Mail::storage))
        .then<void, Sink::ApplicationDomain::SinkResource>([this](const Sink::ApplicationDomain::SinkResource &resource) {
            mIdentifier = resource.identifier();
            mImapServer = resource.getProperty("server").toString();
            mImapUsername = resource.getProperty("username").toString();
            mImapPassword = resource.getProperty("password").toString();
            emit imapResourceChanged();
        },
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Failed to find the imap resource: " << errorMessage;
        }).exec();

    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkResource>(Sink::Query::AccountFilter(id) + Sink::Query::CapabilityFilter(Sink::ApplicationDomain::ResourceCapabilities::Mail::transport))
        .then<void, Sink::ApplicationDomain::SinkResource>([this](const Sink::ApplicationDomain::SinkResource &resource) {
            mMailtransportIdentifier = resource.identifier();
            mSmtpServer = resource.getProperty("server").toString();
            mSmtpUsername = resource.getProperty("username").toString();
            mSmtpPassword = resource.getProperty("password").toString();
            emit smtpResourceChanged();
        },
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Failed to find the smtp resource: " << errorMessage;
        }).exec();

    //FIXME this assumes that we only ever have one identity per account
    Sink::Store::fetchOne<Sink::ApplicationDomain::Identity>(Sink::Query::AccountFilter(id))
        .then<void, Sink::ApplicationDomain::Identity>([this](const Sink::ApplicationDomain::Identity &identity) {
            mIdentityIdentifier = identity.identifier();
            mUsername = identity.getProperty("username").toString();
            mEmailAddress = identity.getProperty("address").toString();
            emit identityChanged();
        },
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Failed to find the identity resource: " << errorMessage;
        }).exec();
}

QByteArray ImapSettings::accountIdentifier() const
{
    return mAccountIdentifier;
}

QValidator *ImapSettings::imapServerValidator() const
{
    class ImapServerValidator : public QValidator {
        State validate(QString &input, int &pos) const {
            Q_UNUSED(pos);
            // imaps://mainserver.example.net:475
            const QUrl url(input);
            static QSet<QString> validProtocols = QSet<QString>() << "imap" << "imaps";
            if (url.isValid() && validProtocols.contains(url.scheme().toLower())) {
                return Acceptable;
            } else {
                return Intermediate;
            }
        }
    };
    static ImapServerValidator *validator = new ImapServerValidator;
    return validator;
}

QValidator *ImapSettings::smtpServerValidator() const
{
    class SmtpServerValidator : public QValidator {
        State validate(QString &input, int &pos) const {
            Q_UNUSED(pos);
            // smtps://mainserver.example.net:475
            const QUrl url(input);
            static QSet<QString> validProtocols = QSet<QString>() << "smtp" << "smtps";
            if (url.isValid() && validProtocols.contains(url.scheme().toLower())) {
                return Acceptable;
            } else {
                return Intermediate;
            }
        }
    };
    static SmtpServerValidator *validator = new SmtpServerValidator;
    return validator;
}

void ImapSettings::save()
{
    qDebug() << "Saving account " << mAccountIdentifier << mIdentifier << mMailtransportIdentifier;
    Q_ASSERT(!mAccountIdentifier.isEmpty());
    Sink::ApplicationDomain::SinkAccount account(mAccountIdentifier);
    account.setProperty("type", "imap");
    account.setProperty("name", mName);
    account.setProperty("icon", mIcon);
    Q_ASSERT(!account.identifier().isEmpty());
    Sink::Store::modify(account).then<void>([]() {},
    [](int errorCode, const QString &errorMessage) {
        qWarning() << "Error while creating account: " << errorMessage;
    })
    .exec();

    if (!mIdentifier.isEmpty()) {
        Sink::ApplicationDomain::SinkResource resource(mIdentifier);
        resource.setProperty("server", mImapServer);
        resource.setProperty("username", mImapUsername);
        resource.setProperty("password", mImapPassword);
        Sink::Store::modify(resource).then<void>([](){}, [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while modifying resource: " << errorMessage;
        })
        .exec();
    } else {
        auto resource = Sink::ApplicationDomain::ImapResource::create(mAccountIdentifier);
        mIdentifier = resource.identifier();
        resource.setProperty("server", mImapServer);
        resource.setProperty("username", mImapUsername);
        resource.setProperty("password", mImapPassword);
        Sink::Store::create(resource).then<void>([]() {},
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while creating resource: " << errorMessage;
        })
        .exec();
    }

    if (!mMailtransportIdentifier.isEmpty()) {
        Sink::ApplicationDomain::SinkResource resource(mMailtransportIdentifier);
        resource.setProperty("server", mSmtpServer);
        resource.setProperty("username", mSmtpUsername);
        resource.setProperty("password", mSmtpPassword);
        Sink::Store::modify(resource).then<void>([](){}, [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while modifying resource: " << errorMessage;
        })
        .exec();
    } else {
        auto resource = Sink::ApplicationDomain::MailtransportResource::create(mAccountIdentifier);
        mMailtransportIdentifier = resource.identifier();
        resource.setProperty("server", mSmtpServer);
        resource.setProperty("username", mSmtpUsername);
        resource.setProperty("password", mSmtpPassword);
        Sink::Store::create(resource).then<void>([]() {},
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while creating resource: " << errorMessage;
        })
        .exec();
    }

    if (!mIdentityIdentifier.isEmpty()) {
        Sink::ApplicationDomain::Identity identity(mMailtransportIdentifier);
        identity.setProperty("username", mUsername);
        identity.setProperty("address", mEmailAddress);
        Sink::Store::modify(identity).then<void>([](){}, [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while modifying identity: " << errorMessage;
        })
        .exec();
    } else {
        auto identity = Sink::ApplicationDomain::ApplicationDomainType::createEntity<Sink::ApplicationDomain::Identity>();
        mIdentityIdentifier = identity.identifier();
        identity.setProperty("account", mAccountIdentifier);
        identity.setProperty("username", mUsername);
        identity.setProperty("address", mEmailAddress);
        Sink::Store::create(identity).then<void>([]() {},
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while creating identity: " << errorMessage;
        })
        .exec();
    }
}

void ImapSettings::remove()
{
    if (mIdentifier.isEmpty()) {
        qWarning() << "We're missing an identifier";
    } else {
        Sink::ApplicationDomain::SinkResource mailTransportResource("", mMailtransportIdentifier, 0, QSharedPointer<Sink::ApplicationDomain::MemoryBufferAdaptor>::create());
        Sink::Store::remove(mailTransportResource).then<void>([]() {},
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while removing resource: " << errorMessage;
        })
        .exec();

        Sink::ApplicationDomain::SinkResource resource("", mIdentifier, 0, QSharedPointer<Sink::ApplicationDomain::MemoryBufferAdaptor>::create());
        Sink::Store::remove(resource).then<void>([]() {},
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while removing resource: " << errorMessage;
        })
        .exec();

        Sink::ApplicationDomain::SinkAccount account("", mAccountIdentifier, 0, QSharedPointer<Sink::ApplicationDomain::MemoryBufferAdaptor>::create());
        Sink::Store::remove(account).then<void>([]() {},
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while removing account: " << errorMessage;
        })
        .exec();
    }
}

