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

#pragma once

#include "maillistmodel.h"

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QScopedPointer>

class SingleMailController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (MailListModel *model READ model CONSTANT)

public:
    explicit SingleMailController(QObject *parent = Q_NULLPTR);
    ~SingleMailController();

    MailListModel *model() const;

Q_SIGNALS:
    void messageChanged();

public slots:
    void loadMail(const QString &id);

private:
    QScopedPointer<MailListModel> m_model;
};
