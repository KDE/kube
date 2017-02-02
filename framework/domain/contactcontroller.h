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

#include <QObject>
#include <QString>
#include <QStringList>

#include "controller.h"

class ContactController : public Kube::Controller
{
    Q_OBJECT

    Q_PROPERTY(QStringList emails READ emails NOTIFY emailsChanged)

    //Interface properties
    KUBE_CONTROLLER_PROPERTY(QString, Name, name)

    KUBE_CONTROLLER_ACTION(save)

public:
    explicit ContactController();

    QStringList emails() const;

    Q_INVOKABLE void loadContact(const QVariant &contact);
    Q_INVOKABLE void removeEmail(const QString &email);
    Q_INVOKABLE void addEmail(const QString &email);

signals:
    void emailsChanged();

private slots:
    void updateSaveAction();

private:
    QStringList m_emails;
};
