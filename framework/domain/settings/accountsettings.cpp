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
#include "accountsettings.h"

#include <sink/store.h>
#include <sink/log.h>
#include <QDebug>
#include <QDir>
#include <QUrl>

using namespace Sink;
using namespace Sink::ApplicationDomain;

SINK_DEBUG_AREA("accountsettings")

AccountSettings::AccountSettings(QObject *parent)
    : QObject(parent)
{
}


void AccountSettings::setAccountIdentifier(const QByteArray &id)
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

    load();

}

QByteArray AccountSettings::accountIdentifier() const
{
    return mAccountIdentifier;
}

void AccountSettings::setPath(const QUrl &path)
{
    auto normalizedPath = path.path();
    if (mPath != normalizedPath) {
        mPath = normalizedPath;
        emit pathChanged();
    }
}

QUrl AccountSettings::path() const
{
    return QUrl(mPath);
}

QValidator *AccountSettings::pathValidator() const
{
    class PathValidator : public QValidator {
        State validate(QString &input, int &pos) const {
            Q_UNUSED(pos);
            if (!input.isEmpty() && QDir(input).exists()) {
                return Acceptable;
            } else {
                return Intermediate;
            }
        }
    };
    static PathValidator *pathValidator = new PathValidator;
    return pathValidator;
}

QValidator *AccountSettings::imapServerValidator() const
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

QValidator *AccountSettings::smtpServerValidator() const
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

void AccountSettings::saveAccount()
{
    qDebug() << "Saving account " << mAccountIdentifier << mMailtransportIdentifier;
    Q_ASSERT(!mAccountIdentifier.isEmpty());
    SinkAccount account(mAccountIdentifier);
    account.setAccountType("imap");
    account.setName(mName);
    account.setIcon(mIcon);
    Q_ASSERT(!account.identifier().isEmpty());
    Store::modify(account)
        .onError([](const KAsync::Error &error) {
            qWarning() << "Error while creating account: " << error.errorMessage;;
        })
        .exec();
}

void AccountSettings::loadAccount()
{
    Q_ASSERT(!mAccountIdentifier.isEmpty());
    Store::fetchOne<SinkAccount>(Query().filter(mAccountIdentifier))
        .then([this](const SinkAccount &account) {
            mIcon = account.getIcon();
            mName = account.getName();
            emit changed();
        }).exec();
}

void AccountSettings::loadImapResource()
{
    Store::fetchOne<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier).containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::storage))
        .then([this](const SinkResource &resource) {
            mImapIdentifier = resource.identifier();
            mImapServer = resource.getProperty("server").toString();
            mImapUsername = resource.getProperty("username").toString();
            mImapPassword = resource.getProperty("password").toString();
            emit imapResourceChanged();
        }).onError([](const KAsync::Error &error) {
            qWarning() << "Failed to find the imap resource: " << error.errorMessage;
        }).exec();
}

void AccountSettings::loadMaildirResource()
{
    Store::fetchOne<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier).containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::storage))
        .then([this](const SinkResource &resource) {
            mMaildirIdentifier = resource.identifier();
            auto path = resource.getProperty("path").toString();
            if (mPath != path) {
                mPath = path;
                emit pathChanged();
            }
        }).onError([](const KAsync::Error &error) {
            SinkWarning() << "Failed to find the maildir resource: " << error.errorMessage;
        }).exec();
}

void AccountSettings::loadMailtransportResource()
{
    Store::fetchOne<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier).containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::transport))
        .then([this](const SinkResource &resource) {
            mMailtransportIdentifier = resource.identifier();
            mSmtpServer = resource.getProperty("server").toString();
            mSmtpUsername = resource.getProperty("username").toString();
            mSmtpPassword = resource.getProperty("password").toString();
            emit smtpResourceChanged();
        }).onError([](const KAsync::Error &error) {
            SinkWarning() << "Failed to find the smtp resource: " << error.errorMessage;
        }).exec();
}

void AccountSettings::loadIdentity()
{
    //FIXME this assumes that we only ever have one identity per account
    Store::fetchOne<Identity>(Query().filter<Identity::Account>(mAccountIdentifier))
        .then([this](const Identity &identity) {
            mIdentityIdentifier = identity.identifier();
            mUsername = identity.getName();
            mEmailAddress = identity.getAddress();
            emit identityChanged();
        }).onError([](const KAsync::Error &error) {
            SinkWarning() << "Failed to find the identity resource: " << error.errorMessage;
        }).exec();
}



template<typename ResourceType>
static QByteArray saveResource(const QByteArray &accountIdentifier, const QByteArray &identifier, const std::map<QByteArray, QVariant> &properties)
{
    if (!identifier.isEmpty()) {
        SinkResource resource(identifier);
        for (const auto &pair : properties) {
            resource.setProperty(pair.first, pair.second);
        }
        Store::modify(resource)
            .onError([](const KAsync::Error &error) {
                SinkWarning() << "Error while modifying resource: " << error.errorMessage;
            })
            .exec();
    } else {
        auto resource = ResourceType::create(accountIdentifier);
        auto newIdentifier = resource.identifier();
        for (const auto &pair : properties) {
            resource.setProperty(pair.first, pair.second);
        }
        Store::create(resource)
            .onError([](const KAsync::Error &error) {
                SinkWarning() << "Error while creating resource: " << error.errorMessage;
            })
            .exec();
        return newIdentifier;
    }
    return identifier;
}

void AccountSettings::saveImapResource()
{
    mImapIdentifier = saveResource<ImapResource>(mAccountIdentifier, mImapIdentifier, {
            {"server", mImapServer},
            {"username", mImapUsername},
            {"password", mImapPassword},
        });
}

void AccountSettings::saveMaildirResource()
{
    mMaildirIdentifier = saveResource<MaildirResource>(mAccountIdentifier, mMaildirIdentifier, {
            {"path", mPath},
        });
}

void AccountSettings::saveMailtransportResource()
{
    mMailtransportIdentifier = saveResource<MailtransportResource>(mAccountIdentifier, mMailtransportIdentifier, {
            {"server", mSmtpServer},
            {"username", mSmtpUsername},
            {"password", mSmtpPassword},
        });
}

void AccountSettings::saveIdentity()
{
    if (!mIdentityIdentifier.isEmpty()) {
        Identity identity(mMailtransportIdentifier);
        identity.setName(mUsername);
        identity.setAddress(mEmailAddress);
        Store::modify(identity)
        .onError([](const KAsync::Error &error) {
            SinkWarning() << "Error while modifying identity: " << error.errorMessage;
        })
        .exec();
    } else {
        auto identity = ApplicationDomainType::createEntity<Identity>();
        mIdentityIdentifier = identity.identifier();
        identity.setAccount(mAccountIdentifier);
        identity.setName(mUsername);
        identity.setAddress(mEmailAddress);
        Store::create(identity)
        .onError([](const KAsync::Error &error) {
            SinkWarning() << "Error while creating identity: " << error.errorMessage;
        })
        .exec();
    }
}

void AccountSettings::removeResource(const QByteArray &identifier)
{
    if (identifier.isEmpty()) {
        SinkWarning() << "We're missing an identifier";
    } else {
        SinkResource resource(identifier);
        Store::remove(resource)
        .onError([](const KAsync::Error &error) {
            SinkWarning() << "Error while removing resource: " << error.errorMessage;
        })
        .exec();
    }
}

void AccountSettings::removeAccount()
{
    if (mAccountIdentifier.isEmpty()) {
        SinkWarning() << "We're missing an identifier";
    } else {
        SinkAccount account(mAccountIdentifier);
        Store::remove(account)
        .onError([](const KAsync::Error &error) {
            SinkWarning() << "Error while removing account: " << error.errorMessage;
        })
        .exec();
    }
}

void AccountSettings::removeIdentity()
{
    if (mIdentityIdentifier.isEmpty()) {
        SinkWarning() << "We're missing an identifier";
    } else {
        Identity identity(mIdentityIdentifier);
        Store::remove(identity)
        .onError([](const KAsync::Error &error) {
            SinkWarning() << "Error while removing identity: " << error.errorMessage;
        })
        .exec();
    }
}

