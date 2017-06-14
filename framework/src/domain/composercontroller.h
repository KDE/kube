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
#include <QStringListModel>
#include <QStandardItemModel>
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

    Q_PROPERTY (QAbstractItemModel* toModel READ toModel CONSTANT)
    Q_PROPERTY (QAbstractItemModel* ccModel READ ccModel CONSTANT)
    Q_PROPERTY (QAbstractItemModel* bccModel READ bccModel CONSTANT)
    Q_PROPERTY (QAbstractItemModel* attachmentModel READ attachmentModel CONSTANT)

    KUBE_CONTROLLER_ACTION(send)
    KUBE_CONTROLLER_ACTION(saveAsDraft)

public:
    explicit ComposerController();

    Completer *recipientCompleter() const;
    Selector *identitySelector() const;

    Q_INVOKABLE void loadMessage(const QVariant &draft, bool loadAsDraft);

    QAbstractItemModel *toModel() const;
    QAbstractItemModel *ccModel() const;
    QAbstractItemModel *bccModel() const;
    QAbstractItemModel *attachmentModel() const;

    Q_INVOKABLE void addTo(const QString &);
    Q_INVOKABLE void removeTo(const QString &);
    Q_INVOKABLE void addCc(const QString &);
    Q_INVOKABLE void removeCc(const QString &);
    Q_INVOKABLE void addBcc(const QString &);
    Q_INVOKABLE void removeBcc(const QString &);
    Q_INVOKABLE void addAttachment(const QUrl &);
    Q_INVOKABLE void removeAttachment(const QUrl &);

public slots:
    virtual void clear() Q_DECL_OVERRIDE;

private slots:
    void updateSendAction();
    void updateSaveAsDraftAction();

private:
    enum AttachmentRoles {
        NameRole = Qt::UserRole + 1,
        FilenameRole,
        ContentRole,
        MimeTypeRole,
        DescriptionRole,
        InlineRole,
        IconNameRole,
        UrlRole
    };

    void recordForAutocompletion(const QByteArray &addrSpec, const QByteArray &displayName);
    void setMessage(const QSharedPointer<KMime::Message> &msg);
    void addAttachmentPart(KMime::Content *partToAttach);
    KMime::Message::Ptr assembleMessage();

    QScopedPointer<Completer> mRecipientCompleter;
    QScopedPointer<Selector> mIdentitySelector;
    QScopedPointer<QStringListModel> mToModel;
    QScopedPointer<QStringListModel> mCcModel;
    QScopedPointer<QStringListModel> mBccModel;
    QScopedPointer<QStandardItemModel> mAttachmentModel;
};
