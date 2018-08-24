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
#include "gmailsettings.h"

GmailSettings::GmailSettings(QObject *parent)
    : AccountSettings(parent)
{
}

void GmailSettings::load()
{
    loadAccount();
    loadImapResource();
    loadMailtransportResource();
    loadIdentity();
}

void GmailSettings::save()
{
    mImapServer = "imaps://imap.gmail.com:993";
    mImapUsername = mEmailAddress;

    mSmtpServer = "smtps://smtp.gmail.com:587";
    mSmtpUsername = mEmailAddress;

    mCardDavServer = QStringLiteral("https://www.googleapis.com/carddav/v1/principals/") + mEmailAddress + "/";
    mCardDavUsername = mEmailAddress;

    mCalDavServer = QStringLiteral("https://www.google.com/calendar/dav/") + mEmailAddress + "/";
    mCalDavUsername = mEmailAddress;

    saveAccount();
    saveImapResource();
    saveMailtransportResource();
    saveCardDavResource();
    saveCalDavResource();
    saveIdentity();
}

void GmailSettings::remove()
{
    removeResource(mMailtransportIdentifier);
    removeResource(mImapIdentifier);
    removeIdentity();
    removeAccount();
}

