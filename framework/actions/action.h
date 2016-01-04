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
#pragma once

#include <QObject>
#include "context.h"

namespace Kube {

class Action : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray actionId READ actionId WRITE setActionId)
    //FIXME if I set the property to Context* qml fails to assign the registered type which is calle Kube::Context_QML_90 in QML...
    Q_PROPERTY(Context* context READ context WRITE setContext)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

public:
    Action(QObject *parent = 0);

    void setContext(Context *);
    Context *context() const;

    void setActionId(const QByteArray &);
    QByteArray actionId() const;

    bool ready() const;

    Q_INVOKABLE void execute();

Q_SIGNALS:
    void readyChanged();

private Q_SLOTS:
    void contextChanged();

private:
    Context *mContext;
    QByteArray mActionId;
};

}
