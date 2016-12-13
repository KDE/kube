/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>

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
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QAbstractItemModel>
#include <sink/applicationdomaintype.h>

#include <actions/context.h>
#include <actions/action.h>

namespace KMime {
class Message;
}

class ComposerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (Kube::Context* mailContext READ mailContext WRITE setMailContext)
    Q_PROPERTY (int currentIdentityIndex READ currentIdentityIndex WRITE setCurrentIdentityIndex)

    Q_PROPERTY (QString recepientSearchString READ recepientSearchString WRITE setRecepientSearchString)
    Q_PROPERTY (QAbstractItemModel* recepientAutocompletionModel READ recepientAutocompletionModel CONSTANT)
    Q_PROPERTY (QAbstractItemModel* identityModel READ identityModel CONSTANT)

    Q_PROPERTY (Kube::Action* sendAction READ sendAction)
    Q_PROPERTY (Kube::Action* saveAsDraftAction READ saveAsDraftAction)

public:
    explicit ComposerController(QObject *parent = Q_NULLPTR);

    Kube::Context* mailContext() const;
    void setMailContext(Kube::Context *context);

    QString recepientSearchString() const;
    void setRecepientSearchString(const QString &body);

    QAbstractItemModel *identityModel() const;
    QAbstractItemModel *recepientAutocompletionModel() const;

    Q_INVOKABLE void loadMessage(const QVariant &draft, bool loadAsDraft);

    Kube::Action* sendAction();
    Kube::Action* saveAsDraftAction();

    void setCurrentIdentityIndex(int index);
    int currentIdentityIndex() const;

public slots:
    void clear();

private:
    Kube::ActionHandler *messageHandler();
    void recordForAutocompletion(const QByteArray &addrSpec, const QByteArray &displayName);
    void setMessage(const QSharedPointer<KMime::Message> &msg);

    int m_currentAccountIndex = -1;
    Kube::Context *mContext;
};
