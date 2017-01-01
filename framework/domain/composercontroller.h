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

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <sink/applicationdomaintype.h>
#include <KMime/Message>

#include "completer.h"
#include "selector.h"
#include "controller.h"

inline bool operator !=(const KMime::Types::Mailbox &l, const KMime::Types::Mailbox &r)
{
    return !(l.prettyAddress() == r.prettyAddress());
}

Q_DECLARE_METATYPE(KMime::Types::Mailbox);

namespace KMime {
class Message;
}

class ComposerController : public Kube::Controller
{
    Q_OBJECT

    //Interface properties
    KUBE_CONTROLLER_PROPERTY(QString, To, to)
    KUBE_CONTROLLER_PROPERTY(QString, Cc, cc)
    KUBE_CONTROLLER_PROPERTY(QString, Bcc, bcc)
    KUBE_CONTROLLER_PROPERTY(QString, Subject, subject)
    KUBE_CONTROLLER_PROPERTY(QString, Body, body)

    //Set by identitySelector
    KUBE_CONTROLLER_PROPERTY(KMime::Types::Mailbox, Identity, identity)
    KUBE_CONTROLLER_PROPERTY(QByteArray, AccountId, accountId)

    //Set by loadMessage
    KUBE_CONTROLLER_PROPERTY(KMime::Message::Ptr, ExistingMessage, existingMessage)
    KUBE_CONTROLLER_PROPERTY(Sink::ApplicationDomain::Mail, ExistingMail, existingMail)

    Q_PROPERTY (Completer* recipientCompleter READ recipientCompleter CONSTANT)
    Q_PROPERTY (Selector* identitySelector READ identitySelector CONSTANT)
    //Q_PROPERTY (QValidator* subjectValidator READ subjectValidator CONSTANT)

    Q_PROPERTY (Kube::ControllerAction* sendAction READ sendAction CONSTANT)
    Q_PROPERTY (Kube::ControllerAction* saveAsDraftAction READ saveAsDraftAction CONSTANT)

public:
    explicit ComposerController();

    Completer *recipientCompleter() const;
    Selector *identitySelector() const;

    Q_INVOKABLE void loadMessage(const QVariant &draft, bool loadAsDraft);

    Kube::ControllerAction* sendAction();
    Kube::ControllerAction* saveAsDraftAction();

private slots:
    void updateSendAction();
    void send();
    void updateSaveAsDraftAction();
    void saveAsDraft();

private:
    void recordForAutocompletion(const QByteArray &addrSpec, const QByteArray &displayName);
    void setMessage(const QSharedPointer<KMime::Message> &msg);
    KMime::Message::Ptr assembleMessage();

    QScopedPointer<Kube::ControllerAction> mSendAction;
    QScopedPointer<Kube::ControllerAction> mSaveAsDraftAction;
    QScopedPointer<Completer> mRecipientCompleter;
    QScopedPointer<Selector> mIdentitySelector;
};
