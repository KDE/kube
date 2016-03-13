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

MaildirSettings::MaildirSettings(QObject *parent)
    : QObject(parent)
{
}

void MaildirSettings::setIdentifier(const QByteArray &id)
{
    mIdentifier = id;
    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkResource>(Sink::Query::IdentityFilter(mIdentifier) + Sink::Query::RequestedProperties(QList<QByteArray>() << "path"))
        .then<void, Sink::ApplicationDomain::SinkResource>([this](const Sink::ApplicationDomain::SinkResource &resource) {
            auto path = resource.getProperty("path").toString();
            if (mPath != path) {
                mPath = path;
                emit pathChanged();
            }
        }).exec();
}

QByteArray MaildirSettings::identifier() const
{
    return mIdentifier;
}

void MaildirSettings::setAccountIdentifier(const QByteArray &id)
{
    if (id.isEmpty()) {
        return;
    }
    mAccountIdentifier = id;
    Q_ASSERT(!id.isEmpty());
    Kube::Account account(id);
    auto maildirResource = account.property("maildirResource").toByteArray();
    setIdentifier(maildirResource);
}

QByteArray MaildirSettings::accountIdentifier() const
{
    return mAccountIdentifier;
}

void MaildirSettings::setPath(const QString &path)
{
    if (mPath != path) {
        mPath = path;
        emit pathChanged();
    }
}

QString MaildirSettings::path() const
{
    return mPath;
}

QValidator *MaildirSettings::pathValidator() const
{
    class PathValidator : public QValidator {
        State validate(QString &input, int &pos) const {
            Q_UNUSED(pos);
            if (QDir(input).exists()) {
                return Acceptable;
            } else {
                return Intermediate;
            }
        }
    };
    static PathValidator *pathValidator = new PathValidator;
    return pathValidator;
}

void MaildirSettings::save()
{
    if (!QDir(mPath).exists()) {
        qWarning() << "The path doesn't exist: " << mPath;
        return;
    }
    if (!mIdentifier.isEmpty()) {
        Sink::ApplicationDomain::SinkResource resource(mIdentifier);
        resource.setProperty("path", mPath);
        Sink::Store::modify(resource).exec();
    } else {
        const auto resourceIdentifier = "org.kde.maildir." + QUuid::createUuid().toByteArray();
        mIdentifier = resourceIdentifier;

        Sink::ApplicationDomain::SinkResource resource;
        resource.setProperty("path", property("path"));
        resource.setProperty("identifier", resourceIdentifier);
        resource.setProperty("type", "org.kde.maildir");
        Sink::Store::create(resource).exec();
        Q_ASSERT(!mAccountIdentifier.isEmpty());
        Kube::Account account(mAccountIdentifier);
        account.setProperty("maildirResource", resourceIdentifier);
        account.save();
        //TODO deal with errors while creating
    }
}

void MaildirSettings::remove()
{
    if (!mIdentifier.isEmpty()) {
        qWarning() << "We're missing an identifier";
    } else {
        Sink::ApplicationDomain::SinkResource resource(mIdentifier);
        Sink::Store::remove(resource).exec();
        Kube::Account account(mAccountIdentifier);
        account.remove();

        Kube::Settings settings("accounts");
        auto accounts = settings.property("accounts").toStringList();
        accounts.removeAll(mAccountIdentifier);
        settings.setProperty("accounts", accounts);
        settings.save();
        //TODO deal with errors while removing
    }
}

