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

#include "mailtemplates.h"

ComposerController::ComposerController(QObject *parent) : QObject(parent)
{
    m_identityModel << "Kuberich <kuberich@kolabnow.com>" << "Uni <kuberich@university.edu>" << "Spam <hello.spam@spam.to>";
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

QStringList ComposerController::identityModel() const
{
    return m_identityModel;
}

int ComposerController::fromIndex() const
{
    return m_fromIndex;
}

void ComposerController::setFromIndex(int fromIndex)
{
    if(m_fromIndex != fromIndex) {
        m_fromIndex = fromIndex;
        emit fromIndexChanged();
    }
}

QVariant ComposerController::originalMessage() const
{
    return m_originalMessage;
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
    mail->subject(true)->fromUnicodeString(m_subject, "utf-8");
    mail->setBody(m_body.toUtf8());
    mail->assemble();
    return mail;
}

void ComposerController::send()
{
    auto mail = assembleMessage();
    Kube::ApplicationContext settings;
    auto account = settings.currentAccount();
    auto identity = account.primaryIdentity();
    auto transport = identity.transport();

    Kube::Context context;
    context.setProperty("message", QVariant::fromValue(mail));

    context.setProperty("username", transport.username());
    context.setProperty("password", transport.password());
    context.setProperty("server", transport.server());

    Kube::Action("org.kde.kube.actions.sendmail", context).execute();
    clear();
}

void ComposerController::saveAsDraft()
{
    auto mail = assembleMessage();
    Kube::Context context;
    context.setProperty("message", QVariant::fromValue(mail));
    Kube::Action("org.kde.kube.actions.saveasdraft", context).execute();
    clear();
}

void ComposerController::clear()
{
    setSubject("");
    setBody("");
    setTo("");
    setCc("");
    setBcc("");
    setFromIndex(-1);
}
