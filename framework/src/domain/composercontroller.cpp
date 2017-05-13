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


#include "composercontroller.h"
#include <settings/settings.h>
#include <KMime/Message>
#include <KCodecs/KEmailAddress>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <QList>
#include <QDebug>
#include <QQmlEngine>
#include <sink/store.h>
#include <sink/log.h>

#include "identitiesmodel.h"
#include "recepientautocompletionmodel.h"
#include "mailtemplates.h"

class IdentitySelector : public Selector {
public:
    IdentitySelector(ComposerController &controller) : Selector(new IdentitiesModel), mController(controller)
    {
    }

    void setCurrent(const QModelIndex &index) Q_DECL_OVERRIDE
    {
        if (index.isValid()) {
            auto currentAccountId = index.data(IdentitiesModel::AccountId).toByteArray();

            KMime::Types::Mailbox mb;
            mb.setName(index.data(IdentitiesModel::Username).toString());
            mb.setAddress(index.data(IdentitiesModel::Address).toString().toUtf8());
            SinkLog() << "Setting current identity: " << mb.prettyAddress() << "Account: " << currentAccountId;

            mController.setIdentity(mb);
            mController.setAccountId(currentAccountId);
        } else {
            SinkWarning() << "No valid identity for index: " << index;
            mController.clearIdentity();
            mController.clearAccountId();
        }

    }
private:
    ComposerController &mController;
};

class RecipientCompleter : public Completer {
public:
    RecipientCompleter() : Completer(new RecipientAutocompletionModel)
    {
    }

    void setSearchString(const QString &s) {
        static_cast<RecipientAutocompletionModel*>(model())->setFilter(s);
        Completer::setSearchString(s);
    }
};


ComposerController::ComposerController()
    : Kube::Controller(),
    action_send{new Kube::ControllerAction{this, &ComposerController::send}},
    action_saveAsDraft{new Kube::ControllerAction{this, &ComposerController::saveAsDraft}},
    mRecipientCompleter{new RecipientCompleter},
    mIdentitySelector{new IdentitySelector{*this}}
{
    updateSaveAsDraftAction();
    // mSendAction->monitorProperty<To>();
    // mSendAction->monitorProperty<Send>([] (const QString &) -> bool{
    //     //validate
    // });
    // registerAction<ControllerAction>(&ComposerController::send);
    // actionDepends<ControllerAction, To, Subject>();
    // TODO do in constructor

    QObject::connect(this, &ComposerController::toChanged, &ComposerController::updateSendAction);
    QObject::connect(this, &ComposerController::subjectChanged, &ComposerController::updateSendAction);
    QObject::connect(this, &ComposerController::accountIdChanged, &ComposerController::updateSendAction);
    QObject::connect(this, &ComposerController::toChanged, &ComposerController::updateSaveAsDraftAction);
    QObject::connect(this, &ComposerController::subjectChanged, &ComposerController::updateSaveAsDraftAction);
    QObject::connect(this, &ComposerController::accountIdChanged, &ComposerController::updateSaveAsDraftAction);
    updateSendAction();
}

void ComposerController::clear()
{
    Controller::clear();
    //Reapply account and identity from selection
    mIdentitySelector->reapplyCurrentIndex();
}

Completer *ComposerController::recipientCompleter() const
{
    return mRecipientCompleter.data();
}

Selector *ComposerController::identitySelector() const
{
    return mIdentitySelector.data();
}

void ComposerController::setMessage(const KMime::Message::Ptr &msg)
{
    setTo(msg->to(true)->asUnicodeString());
    setCc(msg->cc(true)->asUnicodeString());
    setSubject(msg->subject(true)->asUnicodeString());
    setBody(msg->body());
    setExistingMessage(msg);
}

void ComposerController::loadMessage(const QVariant &message, bool loadAsDraft)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    Query query(*message.value<Mail::Ptr>());
    query.request<Mail::MimeMessage>();
    Store::fetchOne<Mail>(query).then([this, loadAsDraft](const Mail &mail) {
        setExistingMail(mail);

        //TODO this should probably happen as reaction to the property being set.
        const auto mailData = KMime::CRLFtoLF(mail.getMimeMessage());
        if (!mailData.isEmpty()) {
            KMime::Message::Ptr mail(new KMime::Message);
            mail->setContent(mailData);
            mail->parse();
            if (loadAsDraft) {
                setMessage(mail);
            } else {
                MailTemplates::reply(mail, [this] (const KMime::Message::Ptr &reply) {
                    //We assume reply
                    setMessage(reply);
                });
            }
        } else {
            qWarning() << "Retrieved empty message";
        }
    }).exec();
}

void ComposerController::recordForAutocompletion(const QByteArray &addrSpec, const QByteArray &displayName)
{
    if (auto model = static_cast<RecipientAutocompletionModel*>(recipientCompleter()->model())) {
        model->addEntry(addrSpec, displayName);
    }
}

void applyAddresses(const QString &list, std::function<void(const QByteArray &, const QByteArray &)> callback)
{
    for (const auto &to : KEmailAddress::splitAddressList(list)) {
        QByteArray displayName;
        QByteArray addrSpec;
        QByteArray comment;
        KEmailAddress::splitAddress(to.toUtf8(), displayName, addrSpec, comment);
        callback(addrSpec, displayName);
    }
}

KMime::Message::Ptr ComposerController::assembleMessage()
{
    auto mail = mExistingMessage;
    if (!mail) {
        mail = KMime::Message::Ptr::create();
    }
    applyAddresses(getTo(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
        mail->to(true)->addAddress(addrSpec, displayName);
        recordForAutocompletion(addrSpec, displayName);
    });
    applyAddresses(getCc(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
        mail->cc(true)->addAddress(addrSpec, displayName);
        recordForAutocompletion(addrSpec, displayName);
    });
    applyAddresses(getBcc(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
        mail->bcc(true)->addAddress(addrSpec, displayName);
        recordForAutocompletion(addrSpec, displayName);
    });

    mail->from(true)->addAddress(getIdentity());

    mail->subject(true)->fromUnicodeString(getSubject(), "utf-8");
    mail->setBody(getBody().toUtf8());
    if (!mail->messageID()) {
        mail->messageID(true)->generate("org.kde.kube");
    }
    if (!mail->date(true)->dateTime().isValid()) {
        mail->date(true)->setDateTime(QDateTime::currentDateTimeUtc());
    }

    mail->assemble();
    return mail;
}

void ComposerController::updateSendAction()
{
    auto enabled = !getTo().isEmpty() && !getSubject().isEmpty() && !getAccountId().isEmpty();
    sendAction()->setEnabled(enabled);
}

void ComposerController::send()
{
    // verify<To, Subject>()
    // && verify<Subject>();
    auto message = assembleMessage();

    auto accountId = getAccountId();
    //SinkLog() << "Sending a mail: " << *this;
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    Q_ASSERT(!accountId.isEmpty());
    Query query;
    query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::transport);
    query.filter<SinkResource::Account>(accountId);
    auto job = Store::fetchAll<SinkResource>(query)
        .then([=](const QList<SinkResource::Ptr> &resources) {
            if (!resources.isEmpty()) {
                auto resourceId = resources[0]->identifier();
                SinkLog() << "Sending message via resource: " << resourceId;
                Mail mail(resourceId);
                mail.setMimeMessage(message->encodedContent());
                return Store::create(mail)
                    .then<void>([=] {
                        //Trigger a sync, but don't wait for it.
                        Store::synchronize(Sink::SyncScope{}.resourceFilter(resourceId)).exec();
                    });
            }
            SinkWarning() << "Failed to find a mailtransport resource";
            return KAsync::error<void>(0, "Failed to find a MailTransport resource.");
        })
        .then([&] (const KAsync::Error &error) {
            SinkLog() << "Message was sent: ";
            emit done();
        });
    run(job);
}

void ComposerController::updateSaveAsDraftAction()
{
    bool enabled = !getAccountId().isEmpty();
    sendAction()->setEnabled(enabled);
}

void ComposerController::saveAsDraft()
{
    SinkLog() << "Save as draft";
    const auto accountId = getAccountId();
    auto existingMail = getExistingMail();

    auto message = assembleMessage();
    //FIXME this is something for the validation
    if (!message) {
        SinkWarning() << "Failed to get the mail: ";
        return;
    }

    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    auto job = [&] {
        if (existingMail.identifier().isEmpty()) {
            SinkLog() << "Creating a new draft" << existingMail.identifier();
            Query query;
            query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::drafts);
            query.filter<SinkResource::Account>(accountId);
            return Store::fetchOne<SinkResource>(query)
                .then([=](const SinkResource &resource) {
                    Mail mail(resource.identifier());
                    mail.setDraft(true);
                    mail.setMimeMessage(message->encodedContent());
                    return Store::create(mail);
                })
                .onError([] (const KAsync::Error &error) {
                    SinkWarning() << "Error while creating draft: " << error.errorMessage;
                });
        } else {
            SinkLog() << "Modifying an existing mail" << existingMail.identifier();
            existingMail.setDraft(true);
            existingMail.setMimeMessage(message->encodedContent());
            return Store::modify(existingMail);
        }
    }();
    job = job.then([&] (const KAsync::Error &) {
        emit done();
    });
    run(job);
}
