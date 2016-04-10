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
#include "maildirsettings.h"

#include <settings/settings.h>

#include <sink/store.h>
#include <QDebug>
#include <QUuid>
#include <QDir>
#include <QUrl>

MaildirSettings::MaildirSettings(QObject *parent)
    : QObject(parent)
{
}


void MaildirSettings::setAccountIdentifier(const QByteArray &id)
{
    if (id.isEmpty()) {
        return;
    }
    mAccountIdentifier = id;

    //Clear
    mIcon = QString();
    mName = QString();
    mPath = QString();
    mSmtpServer = QString();
    mSmtpUsername = QString();
    mSmtpPassword = QString();
    emit changed();
    emit pathChanged();
    emit smtpResourceChanged();

    Q_ASSERT(!id.isEmpty());
    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkAccount>(Sink::Query::IdentityFilter(id))
        .then<void, Sink::ApplicationDomain::SinkAccount>([this](const Sink::ApplicationDomain::SinkAccount &account) {
            mIcon = account.getProperty("icon").toString();
            mName = account.getProperty("name").toString();
            emit changed();
        }).exec();

    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkResource>(Sink::Query::PropertyFilter("account", QVariant::fromValue(id)) + Sink::Query::PropertyFilter("type", QString("org.kde.maildir")))
        .then<void, Sink::ApplicationDomain::SinkResource>([this](const Sink::ApplicationDomain::SinkResource &resource) {
            mIdentifier = resource.identifier();
            auto path = resource.getProperty("path").toString();
            if (mPath != path) {
                mPath = path;
                emit pathChanged();
            }
        },
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Failed to find the maildir resource: " << errorMessage;
        }).exec();

    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkResource>(Sink::Query::PropertyFilter("account", QVariant::fromValue(id)) + Sink::Query::PropertyFilter("type", QString("org.kde.mailtransport")))
        .then<void, Sink::ApplicationDomain::SinkResource>([this](const Sink::ApplicationDomain::SinkResource &resource) {
            mMailtransportIdentifier = resource.identifier();
            mSmtpServer = resource.getProperty("server").toString();
            mSmtpUsername = resource.getProperty("username").toString();
            mSmtpPassword = resource.getProperty("password").toString();
            emit smtpResourceChanged();
        },
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Failed to find the maildir resource: " << errorMessage;
        }).exec();
}

QByteArray MaildirSettings::accountIdentifier() const
{
    return mAccountIdentifier;
}

void MaildirSettings::setPath(const QUrl &path)
{
    auto normalizedPath = path.path();
    if (mPath != normalizedPath) {
        mPath = normalizedPath;
        emit pathChanged();
    }
}

QUrl MaildirSettings::path() const
{
    return QUrl(mPath);
}

QValidator *MaildirSettings::pathValidator() const
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

QValidator *MaildirSettings::smtpServerValidator() const
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

void MaildirSettings::save()
{
    if (!QDir(mPath).exists()) {
        qWarning() << "The path doesn't exist: " << mPath;
        return;
    }
    qDebug() << "Saving account " << mAccountIdentifier << mIdentifier << mMailtransportIdentifier;
    Q_ASSERT(!mAccountIdentifier.isEmpty());
    Sink::ApplicationDomain::SinkAccount account(mAccountIdentifier);
    account.setProperty("type", "maildir");
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
        resource.setProperty("path", mPath);
        Sink::Store::modify(resource).then<void>([](){}, [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while modifying resource: " << errorMessage;
        })
        .exec();
    } else {
        const auto resourceIdentifier = "org.kde.maildir." + QUuid::createUuid().toByteArray();
        mIdentifier = resourceIdentifier;

        Sink::ApplicationDomain::SinkResource resource;
        resource.setProperty("path", property("path"));
        resource.setProperty("identifier", resourceIdentifier);
        resource.setProperty("type", "org.kde.maildir");
        resource.setProperty("account", mAccountIdentifier);
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
        //FIXME we shouldn't have to do this magic
        const auto resourceIdentifier = "org.kde.mailtransport." + QUuid::createUuid().toByteArray();
        mMailtransportIdentifier = resourceIdentifier;

        Sink::ApplicationDomain::SinkResource resource;
        resource.setProperty("identifier", resourceIdentifier);
        resource.setProperty("type", "org.kde.mailtransport");
        resource.setProperty("account", mAccountIdentifier);
        resource.setProperty("server", mSmtpServer);
        resource.setProperty("username", mSmtpUsername);
        resource.setProperty("password", mSmtpPassword);
        Sink::Store::create(resource).then<void>([]() {},
        [](int errorCode, const QString &errorMessage) {
            qWarning() << "Error while creating resource: " << errorMessage;
        })
        .exec();
    }
}

void MaildirSettings::remove()
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

