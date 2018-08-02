/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#pragma once
#include "kube_export.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QStandardItemModel>
#include <sink/applicationdomaintype.h>

#include "controller.h"

class KUBE_EXPORT ContactController : public Kube::Controller
{
    Q_OBJECT

    // Input properties
    Q_PROPERTY(QVariant contact READ contact WRITE loadContact)

    //Interface properties
    KUBE_CONTROLLER_PROPERTY(QByteArray, AccountId, accountId)
    KUBE_CONTROLLER_PROPERTY(QString, FirstName, firstName)
    KUBE_CONTROLLER_PROPERTY(QString, LastName, lastName)
    KUBE_CONTROLLER_PROPERTY(QString, Name, name)
    KUBE_CONTROLLER_PROPERTY(QString, Street, street)
    KUBE_CONTROLLER_PROPERTY(QString, City, city)
    KUBE_CONTROLLER_PROPERTY(QString, Country, country)
    KUBE_CONTROLLER_PROPERTY(QString, Company, company)
    KUBE_CONTROLLER_PROPERTY(QString, JobTitle, jobTitle)
    KUBE_CONTROLLER_PROPERTY(QByteArray, ImageData, imageData)
    KUBE_CONTROLLER_PROPERTY(Sink::ApplicationDomain::ApplicationDomainType::Ptr, Addressbook, addressbook)

    KUBE_CONTROLLER_LISTCONTROLLER(mails)
    KUBE_CONTROLLER_LISTCONTROLLER(phones)

    KUBE_CONTROLLER_ACTION(save)

public:
    explicit ContactController();

    Q_INVOKABLE void loadContact(const QVariant &contact);
    Q_INVOKABLE void remove();

    QVariant contact() const;

private slots:
    void updateSaveAction();

private:
    QVariant mContact;
};
