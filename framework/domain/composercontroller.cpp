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
#include <settings/settings.h>
#include <KMime/Message>
#include <KCodecs/KEmailAddress>
#include <QVariant>
#include <QDebug>
#include <QQmlEngine>

#include "accountsmodel.h"
#include "identitiesmodel.h"
#include "mailtemplates.h"

ComposerController::ComposerController(QObject *parent) : QObject(parent)
{
}

QString ComposerController::to() const
{
    return m_to;
}

void ComposerController::setTo(const QString &to)
{
    if(m_to != to) {
        m_to = to;
        emit toChanged();
    }
}

QString ComposerController::cc() const
{
    return m_cc;
}

void ComposerController::setCc(const QString &cc)
{
    if(m_cc != cc) {
        m_cc = cc;
        emit ccChanged();
    }
}

QString ComposerController::bcc() const
{
    return m_bcc;
}

void ComposerController::setBcc(const QString &bcc)
{
    if(m_bcc != bcc) {
        m_bcc = bcc;
        emit bccChanged();
    }
}

QString ComposerController::subject() const
{
    return m_subject;
}

void ComposerController::setSubject(const QString &subject)
{
    if(m_subject != subject) {
        m_subject = subject;
        emit subjectChanged();
    }
}

QString ComposerController::body() const
{
    return m_body;
}

void ComposerController::setBody(const QString &body)
{
    if(m_body != body) {
        m_body = body;
        emit bodyChanged();
    }
}

QAbstractItemModel *ComposerController::identityModel() const
{
    static auto model = new IdentitiesModel();
    QQmlEngine::setObjectOwnership(model, QQmlEngine::CppOwnership);
    return model;
}

QStringList ComposerController::attachemts() const
{
    return m_attachments;
}

QVariant ComposerController::originalMessage() const
{
    return m_originalMessage;
}

void ComposerController::addAttachment(const QUrl &fileUrl)
{
    m_attachments.append(fileUrl.toString());
    emit attachmentsChanged();
}

void ComposerController::setOriginalMessage(const QVariant &originalMessage)
{
    const auto mailData = KMime::CRLFtoLF(originalMessage.toByteArray());
    if (!mailData.isEmpty()) {
        KMime::Message::Ptr mail(new KMime::Message);
        mail->setContent(mailData);
        mail->parse();
        auto reply = MailTemplates::reply(mail);
        //We assume reply
        setTo(reply->to(true)->asUnicodeString());
        setCc(reply->cc(true)->asUnicodeString());
        setSubject(reply->subject(true)->asUnicodeString());
        setBody(reply->body());
        m_msg = QVariant::fromValue(reply);
    } else {
        m_msg = QVariant();
    }
}

KMime::Message::Ptr ComposerController::assembleMessage()
{
    auto mail = m_msg.value<KMime::Message::Ptr>();
    if (!mail) {
        mail = KMime::Message::Ptr::create();
    }
    for (const auto &to : KEmailAddress::splitAddressList(m_to)) {
        QByteArray displayName;
        QByteArray addrSpec;
        QByteArray comment;
        KEmailAddress::splitAddress(to.toUtf8(), displayName, addrSpec, comment);
        mail->to(true)->addAddress(addrSpec, displayName);
    }
    auto currentIndex = identityModel()->index(m_currentAccountIndex, 0);
    KMime::Types::Mailbox mb;
    mb.setName(currentIndex.data(IdentitiesModel::Username).toString());
    mb.setAddress(currentIndex.data(IdentitiesModel::Address).toString().toUtf8());
    mail->from(true)->addAddress(mb);
    mail->subject(true)->fromUnicodeString(m_subject, "utf-8");
    mail->setBody(m_body.toUtf8());
    mail->assemble();
    return mail;
}

void ComposerController::send()
{
    auto mail = assembleMessage();
    auto currentAccountId = identityModel()->index(m_currentAccountIndex, 0).data(IdentitiesModel::AccountId).toByteArray();

    Kube::Context context;
    context.setProperty("message", QVariant::fromValue(mail));
    context.setProperty("accountId", QVariant::fromValue(currentAccountId));

    qDebug() << "Current account " << currentAccountId;

    Kube::Action("org.kde.kube.actions.sendmail", context).execute();
    clear();
}

void ComposerController::saveAsDraft()
{
    auto mail = assembleMessage();
    auto currentAccountId = identityModel()->index(m_currentAccountIndex, 0).data(IdentitiesModel::AccountId).toByteArray();

    Kube::Context context;
    context.setProperty("message", QVariant::fromValue(mail));
    context.setProperty("accountId", QVariant::fromValue(currentAccountId));
    Kube::Action("org.kde.kube.actions.save-as-draft", context).execute();
    clear();
}

void ComposerController::clear()
{
    setSubject("");
    setBody("");
    setTo("");
    setCc("");
    setBcc("");
}
