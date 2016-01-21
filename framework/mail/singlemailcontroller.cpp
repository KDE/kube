/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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

#include "singlemailcontroller.h"

SingleMailController::SingleMailController(QObject *parent) : QObject(parent), m_model(new MailListModel)
{

}

SingleMailController::~SingleMailController()
{

}

MailListModel* SingleMailController::model() const
{
    return m_model.data();
}


void SingleMailController::loadMail(const QString &id)
{
    Sink::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = false;
    query.requestedProperties << "subject" << "sender" << "senderName" << "date" << "unread" << "important" << "mimeMessage";
    query.ids << id.toLatin1();
    m_model->runQuery(query);
}