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

#include "keyring.h"
#include <sink/crypto.h>

using namespace Sink;
using namespace Sink::ApplicationDomain;

AccountSettings::AccountSettings(QObject *parent)
    : QObject(parent)
{
}

void AccountSettings::setAccountType(const QByteArray &type)
{
    mAccountType = type;
}

QByteArray AccountSettings::accountType() const
{
    return mAccountType;
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
    mSmtpServer = QString();
    mSmtpUsername = QString();
    mCardDavServer = QString();
    mCardDavUsername = QString();
    mCalDavServer = QString();
    mCalDavUsername = QString();
    mPath = QString();
    emit changed();
    emit imapResourceChanged();
    emit smtpResourceChanged();
    emit cardDavResourceChanged();
    emit calDavResourceChanged();
    emit pathChanged();

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
    if (mAccountIdentifier.isEmpty()) {
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        mAccountIdentifier = account.identifier();
        Q_ASSERT(!mAccountType.isEmpty());
        account.setAccountType(mAccountType);
        account.setName(mName);
        account.setIcon(mIcon);
        Store::create(account)
            .onError([](const KAsync::Error &error) {
                qWarning() << "Error while creating account: " << error.errorMessage;;
            })
            .exec().waitForFinished();
    } else {
        qDebug() << "Saving account " << mAccountIdentifier;
        Q_ASSERT(!mAccountIdentifier.isEmpty());
        SinkAccount account(mAccountIdentifier);
        account.setAccountType(mAccountType);
        account.setName(mName);
        account.setIcon(mIcon);
        Q_ASSERT(!account.identifier().isEmpty());
        Store::modify(account)
            .onError([](const KAsync::Error &error) {
                qWarning() << "Error while creating account: " << error.errorMessage;;
            })
            .exec().waitForFinished();
    }
}

void AccountSettings::loadAccount()
{
    Q_ASSERT(!mAccountIdentifier.isEmpty());
    Store::fetchOne<SinkAccount>(Query().filter(mAccountIdentifier).request<SinkAccount::Icon>().request<SinkAccount::Name>().request<SinkAccount::AccountType>())
        .then([this](const SinkAccount &account) {
            mAccountType = account.getAccountType().toLatin1();
            mIcon = account.getIcon();
            mName = account.getName();
            emit changed();
        }).onError([](const KAsync::Error &error) {
            qWarning() << "Failed to load the account: " << error.errorMessage;
        }).exec().waitForFinished();
}

void AccountSettings::loadImapResource()
{
    Store::fetchOne<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier).filter<SinkResource::ResourceType>("sink.imap"))
        .then([this](const SinkResource &resource) {
            mImapIdentifier = resource.identifier();
            mImapServer = resource.getProperty("server").toString();
            mImapUsername = resource.getProperty("username").toString();
            emit imapResourceChanged();
        }).onError([](const KAsync::Error &error) {
            qWarning() << "Failed to load the imap resource: " << error.errorMessage;
        }).exec().waitForFinished();
}

void AccountSettings::loadMaildirResource()
{
    Store::fetchOne<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier).filter<SinkResource::ResourceType>("sink.maildir"))
        .then([this](const SinkResource &resource) {
            mMaildirIdentifier = resource.identifier();
            mPath = resource.getProperty("path").toString();
            emit pathChanged();
        }).onError([](const KAsync::Error &error) {
            SinkWarning() << "Failed to load the maildir resource: " << error.errorMessage;
        }).exec().waitForFinished();
}

void AccountSettings::loadMailtransportResource()
{
    Store::fetchOne<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier).filter<SinkResource::ResourceType>("sink.mailtransport"))
        .then([this](const SinkResource &resource) {
            mMailtransportIdentifier = resource.identifier();
            mSmtpServer = resource.getProperty("server").toString();
            mSmtpUsername = resource.getProperty("username").toString();
            emit smtpResourceChanged();
        }).onError([](const KAsync::Error &error) {
            SinkWarning() << "Failed to load the smtp resource: " << error.errorMessage;
        }).exec().waitForFinished();
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
            SinkWarning() << "Failed to load the identity: " << error.errorMessage;
        }).exec().waitForFinished();
}

void AccountSettings::loadCardDavResource()
{
    Store::fetchOne<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier).filter<SinkResource::ResourceType>("sink.carddav"))
        .then([this](const SinkResource &resource) {
            mCardDavIdentifier = resource.identifier();
            mCardDavServer = resource.getProperty("server").toString();
            mCardDavUsername = resource.getProperty("username").toString();
            emit cardDavResourceChanged();
        }).onError([](const KAsync::Error &error) {
            qWarning() << "Failed to load the CardDAV resource: " << error.errorMessage;
        }).exec().waitForFinished();
}

void AccountSettings::loadCalDavResource()
{
    Store::fetchOne<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier).filter<SinkResource::ResourceType>("sink.caldav"))
        .then([this](const SinkResource &resource) {
            mCalDavIdentifier = resource.identifier();
            mCalDavServer = resource.getProperty("server").toString();
            mCalDavUsername = resource.getProperty("username").toString();
            emit calDavResourceChanged();
        }).onError([](const KAsync::Error &error) {
            qWarning() << "Failed to load the CalDAV resource: " << error.errorMessage;
        }).exec().waitForFinished();
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
            .exec().waitForFinished();
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
            .exec().waitForFinished();
        return newIdentifier;
    }
    return identifier;
}

void AccountSettings::saveImapResource()
{
    mImapIdentifier = saveResource<ImapResource>(mAccountIdentifier, mImapIdentifier, {
            {"server", mImapServer},
            {"username", mImapUsername}
        });
}

void AccountSettings::saveCardDavResource()
{
    mCardDavIdentifier = saveResource<CardDavResource>(mAccountIdentifier, mCardDavIdentifier, {
            {"server", mCardDavServer},
            {"username", mCardDavUsername}
        });
}

void AccountSettings::saveCalDavResource()
{
    mCalDavIdentifier = saveResource<CalDavResource>(mAccountIdentifier, mCalDavIdentifier, {
            {"server", mCalDavServer},
            {"username", mCalDavUsername}
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
            {"username", mSmtpUsername}
        });
}

void AccountSettings::login(const QVariantMap &secrets)
{
    // We'll attempt to store your account secrets using a key matching the email address.
    const auto accountSecret = secrets.value("accountSecret").toString();
    Store::fetchAll<SinkResource>(Query().filter<SinkResource::Account>(mAccountIdentifier))
        .then([=](const QList<SinkResource::Ptr> &resources) {
            Kube::AccountKeyring keyring{mAccountIdentifier};
            for (const auto &resource : resources) {
                keyring.addPassword(resource->identifier(), accountSecret);
            }
            const auto keys = Crypto::findKeys({{mEmailAddress}}, true);
            if (!keys.empty()) {
                qInfo() << "Storing account secrets.";
                keyring.save(keys);
            } else {
                qInfo() << "Failed to find a GPG key for " << mEmailAddress << ". Not storing account secrets.";
            }
        }).onError([](const KAsync::Error &error) {
            qWarning() << "Failed to load any account resources resource: " << error;
        }).exec();
}

void AccountSettings::saveIdentity()
{
    if (!mIdentityIdentifier.isEmpty()) {
        Identity identity(mIdentityIdentifier);
        identity.setName(mUsername);
        identity.setAddress(mEmailAddress);
        Store::modify(identity)
        .onError([](const KAsync::Error &error) {
            SinkWarning() << "Error while modifying identity: " << error.errorMessage;
        })
        .exec().waitForFinished();
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
        .exec().waitForFinished();
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
        .exec().waitForFinished();
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
        .exec().waitForFinished();
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
        .exec().waitForFinished();
    }
}

void AccountSettings::load()
{
    loadAccount();
    loadImapResource();
    loadMailtransportResource();
    loadCardDavResource();
    loadCalDavResource();
    loadIdentity();
}

void AccountSettings::save()
{
    saveAccount();
    saveImapResource();
    saveMailtransportResource();
    saveCardDavResource();
    saveCalDavResource();
    saveIdentity();
}

void AccountSettings::remove()
{
    removeResource(mMailtransportIdentifier);
    removeResource(mImapIdentifier);
    removeResource(mCardDavIdentifier);
    removeResource(mCalDavIdentifier);
    removeIdentity();
    removeAccount();
}

