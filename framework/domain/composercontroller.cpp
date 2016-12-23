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


#include "composercontroller.h"
#include <actions/context.h>
#include <actions/action.h>
#include <actions/actionhandler.h>
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

#include "accountsmodel.h"
#include "identitiesmodel.h"
#include "recepientautocompletionmodel.h"
#include "mailtemplates.h"

SINK_DEBUG_AREA("composercontroller");

Q_DECLARE_METATYPE(KMime::Types::Mailbox)

ComposerController::ComposerController(QObject *parent) : QObject(parent)
{
}

QString ComposerController::recepientSearchString() const
{
    return QString();
}

Kube::Context* ComposerController::mailContext() const
{
    return mContext;
}

void ComposerController::setMailContext(Kube::Context *context)
{
    mContext = context;
}

void ComposerController::setRecepientSearchString(const QString &s)
{
    if (auto model = static_cast<RecipientAutocompletionModel*>(recepientAutocompletionModel())) {
        model->setFilter(s);
    }
}

QAbstractItemModel *ComposerController::identityModel() const
{
    static auto model = new IdentitiesModel();
    QQmlEngine::setObjectOwnership(model, QQmlEngine::CppOwnership);
    return model;
}

QAbstractItemModel *ComposerController::recepientAutocompletionModel() const
{
    static auto model = new RecipientAutocompletionModel();
    QQmlEngine::setObjectOwnership(model, QQmlEngine::CppOwnership);
    return model;
}

void ComposerController::setMessage(const KMime::Message::Ptr &msg)
{
    mContext->setProperty("to", msg->to(true)->asUnicodeString());
    mContext->setProperty("cc", msg->cc(true)->asUnicodeString());
    mContext->setProperty("subject", msg->subject(true)->asUnicodeString());
    mContext->setProperty("body", msg->body());
    mContext->setProperty("existingMessage", QVariant::fromValue(msg));
}

void ComposerController::loadMessage(const QVariant &message, bool loadAsDraft)
{
    Sink::Query query(*message.value<Sink::ApplicationDomain::Mail::Ptr>());
    query.request<Sink::ApplicationDomain::Mail::MimeMessage>();
    Sink::Store::fetchOne<Sink::ApplicationDomain::Mail>(query).syncThen<void, Sink::ApplicationDomain::Mail>([this, loadAsDraft](const Sink::ApplicationDomain::Mail &mail) {
        mContext->setProperty("existingMail", QVariant::fromValue(mail));
        const auto mailData = KMime::CRLFtoLF(mail.getMimeMessage());
        if (!mailData.isEmpty()) {
            KMime::Message::Ptr mail(new KMime::Message);
            mail->setContent(mailData);
            mail->parse();
            if (loadAsDraft) {
                auto reply = MailTemplates::reply(mail);
                //We assume reply
                setMessage(reply);
            } else {
                setMessage(mail);
            }
        } else {
            qWarning() << "Retrieved empty message";
        }
    }).exec();
}

void ComposerController::recordForAutocompletion(const QByteArray &addrSpec, const QByteArray &displayName)
{
    if (auto model = static_cast<RecipientAutocompletionModel*>(recepientAutocompletionModel())) {
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

void ComposerController::clear()
{
    mContext->setProperty("subject", QVariant());
    mContext->setProperty("body", QVariant());
    mContext->setProperty("to", QVariant());
    mContext->setProperty("cc", QVariant());
    mContext->setProperty("bcc", QVariant());
}


Kube::ActionHandler *ComposerController::messageHandler()
{
    return new Kube::ActionHandlerHelper(
        [](Kube::Context *context) {
            auto identity = context->property("identity");
            return identity.isValid();
        },
        [this](Kube::Context *context) {
            auto mail = context->property("existingMessage").value<KMime::Message::Ptr>();
            if (!mail) {
                mail = KMime::Message::Ptr::create();
            }
            applyAddresses(context->property("to").toString(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
                mail->to(true)->addAddress(addrSpec, displayName);
                recordForAutocompletion(addrSpec, displayName);
            });
            applyAddresses(context->property("cc").toString(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
                mail->cc(true)->addAddress(addrSpec, displayName);
                recordForAutocompletion(addrSpec, displayName);
            });
            applyAddresses(context->property("bcc").toString(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
                mail->bcc(true)->addAddress(addrSpec, displayName);
                recordForAutocompletion(addrSpec, displayName);
            });

            mail->from(true)->addAddress(context->property("identity").value<KMime::Types::Mailbox>());

            mail->subject(true)->fromUnicodeString(context->property("subject").toString(), "utf-8");
            mail->setBody(context->property("body").toString().toUtf8());
            mail->assemble();

            context->setProperty("message", QVariant::fromValue(mail));
        }
    );
}

Kube::Action* ComposerController::saveAsDraftAction()
{
    auto action = new Kube::Action("org.kde.kube.actions.save-as-draft", *mContext);
    action->addPreHandler(messageHandler());
    action->addPostHandler(new Kube::ActionHandlerHelper(
        [this](Kube::Context *context) {
            emit done();
        }));
    return action;
}

Kube::Action* ComposerController::sendAction()
{
    qWarning() << "send action";
    auto action = new Kube::Action("org.kde.kube.actions.sendmail", *mContext);
    // action->addPreHandler(identityHandler());
    action->addPreHandler(messageHandler());
    // action->addPreHandler(encryptionHandler());
    action->addPostHandler(new Kube::ActionHandlerHelper(
        [this](Kube::Context *context) {
            emit done();
        }));
    return action;
}

void ComposerController::setCurrentIdentityIndex(int index)
{
    m_currentAccountIndex = index;
    auto currentIndex = identityModel()->index(m_currentAccountIndex, 0);
    if (currentIndex.isValid()) {
        auto currentAccountId = currentIndex.data(IdentitiesModel::AccountId).toByteArray();
        KMime::Types::Mailbox mb;
        mb.setName(currentIndex.data(IdentitiesModel::Username).toString());
        mb.setAddress(currentIndex.data(IdentitiesModel::Address).toString().toUtf8());
        SinkLog() << "Setting current identity: " << mb.prettyAddress() << "Account: " << currentAccountId;
        mContext->setProperty("identity", QVariant::fromValue(mb));
        mContext->setProperty("accountId", QVariant::fromValue(currentAccountId));
    } else {
        SinkWarning() << "No valid identity for index: " << index << " out of available in model: " << identityModel()->rowCount();
    }
}

int ComposerController::currentIdentityIndex() const
{
    return m_currentAccountIndex;
}
