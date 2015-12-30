/*
    This file is part of KMail.

    Copyright (c) 2005 David Faure <faure@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2,
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "messageviewersettings.h"
#include <QTimer>

using namespace MessageViewer;

MessageViewerSettings *MessageViewerSettings::mSelf = 0;

MessageViewerSettings *MessageViewerSettings::self()
{
    if (!mSelf) {
        mSelf = new MessageViewerSettings();
        mSelf->load();
    }

    return mSelf;
}

MessageViewerSettings::MessageViewerSettings()
{
    mConfigSyncTimer = new QTimer(this);
    mConfigSyncTimer->setSingleShot(true);
    connect(mConfigSyncTimer, &QTimer::timeout, this, &MessageViewerSettings::slotSyncNow);
}

void MessageViewerSettings::requestSync()
{
    if (!mConfigSyncTimer->isActive()) {
        mConfigSyncTimer->start(0);
    }
}

void MessageViewerSettings::slotSyncNow()
{
    config()->sync();
}

MessageViewerSettings::~MessageViewerSettings()
{
}

