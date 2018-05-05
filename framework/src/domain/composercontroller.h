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
#include "kube_export.h"
#include <QObject>
#include <QString>
#include <QVariant>
#include <QStandardItemModel>
#include <sink/applicationdomaintype.h>
#include <KMime/Message>
#include <mime/mailcrypto.h>

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

class AddresseeModel;

class KUBE_EXPORT ComposerController : public Kube::Controller
{
    Q_OBJECT

    //Interface properties
    KUBE_CONTROLLER_PROPERTY(QString, Subject, subject)
    KUBE_CONTROLLER_PROPERTY(QString, Body, body)
    KUBE_CONTROLLER_PROPERTY(bool, HtmlBody, htmlBody)
    KUBE_CONTROLLER_PROPERTY(bool, Encrypt, encrypt)
    KUBE_CONTROLLER_PROPERTY(bool, Sign, sign)

    //Set by identitySelector
    KUBE_CONTROLLER_PROPERTY(KMime::Types::Mailbox, Identity, identity)
    KUBE_CONTROLLER_PROPERTY(QByteArray, AccountId, accountId)

    //Set by loadMessage
    KUBE_CONTROLLER_PROPERTY(KMime::Message::Ptr, ExistingMessage, existingMessage)
    KUBE_CONTROLLER_PROPERTY(Sink::ApplicationDomain::Mail, ExistingMail, existingMail)

    KUBE_CONTROLLER_PROPERTY(/*std::vector<Crypto::Key>*/QVariant, PersonalKeys, personalKeys)
    KUBE_CONTROLLER_PROPERTY(bool, FoundPersonalKeys, foundPersonalKeys)

    KUBE_CONTROLLER_LISTCONTROLLER(to)
    KUBE_CONTROLLER_LISTCONTROLLER(cc)
    KUBE_CONTROLLER_LISTCONTROLLER(bcc)
    KUBE_CONTROLLER_LISTCONTROLLER(attachments)

    Q_PROPERTY (Completer* recipientCompleter READ recipientCompleter CONSTANT)
    Q_PROPERTY (Selector* identitySelector READ identitySelector CONSTANT)

    KUBE_CONTROLLER_ACTION(send)
    KUBE_CONTROLLER_ACTION(saveAsDraft)

public:
    enum LoadType {
        Draft,
        Reply,
        Forward,
    };
    Q_ENUMS(LoadType);

    explicit ComposerController();

    Completer *recipientCompleter() const;
    Selector *identitySelector() const;

    Q_INVOKABLE void loadDraft(const QVariant &message);
    Q_INVOKABLE void loadReply(const QVariant &message);
    Q_INVOKABLE void loadForward(const QVariant &message);

public slots:
    virtual void clear() Q_DECL_OVERRIDE;

private slots:
    void findPersonalKey();

private:
    void loadMessage(const QVariant &message, std::function<void(const KMime::Message::Ptr&)> callback);

    void recordForAutocompletion(const QByteArray &addrSpec, const QByteArray &displayName);
    void setMessage(const QSharedPointer<KMime::Message> &msg);
    void addAttachmentPart(KMime::Content *partToAttach);
    KMime::Message::Ptr assembleMessage();
    std::vector<Crypto::Key> getRecipientKeys();

    QScopedPointer<Completer> mRecipientCompleter;
    QScopedPointer<Selector> mIdentitySelector;
    bool mRemoveDraft = false;
};
