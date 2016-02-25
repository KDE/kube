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


#include "composer.h"
#include <actions/context.h>
#include <actions/action.h>
#include <settings/settings.h>
#include <KMime/Message>
#include <KCodecs/KEmailAddress>
#include <QVariant>
#include <QDebug>

Composer::Composer(QObject *parent) : QObject(parent)
{
    m_identityModel << "Kuberich <kuberich@kolabnow.com>" << "Uni <kuberich@university.edu>" << "Spam <hello.spam@spam.to>";
}

QString Composer::to() const
{
    return m_to;
}

void Composer::setTo(const QString &to)
{
    if(m_to != to) {
        m_to = to;
        emit toChanged();
    }
}

QString Composer::cc() const
{
    return m_cc;
}

void Composer::setCc(const QString &cc)
{
    if(m_cc != cc) {
        m_cc = cc;
        emit ccChanged();
    }
}

QString Composer::bcc() const
{
    return m_bcc;
}

void Composer::setBcc(const QString &bcc)
{
    if(m_bcc != bcc) {
        m_bcc = bcc;
        emit bccChanged();
    }
}

QString Composer::subject() const
{
    return m_subject;
}

void Composer::setSubject(const QString &subject)
{
    if(m_subject != subject) {
        m_subject = subject;
        emit subjectChanged();
    }
}

QString Composer::body() const
{
    return m_body;
}

void Composer::setBody(const QString &body)
{
    if(m_body != body) {
        m_body = body;
        emit bodyChanged();
    }
}

QStringList Composer::identityModel() const
{
    return m_identityModel;
}

int Composer::fromIndex() const
{
    return m_fromIndex;
}

void Composer::setFromIndex(int fromIndex)
{
    if(m_fromIndex != fromIndex) {
        m_fromIndex = fromIndex;
        emit fromIndexChanged();
    }
}

void Composer::send()
{
    auto mail = KMime::Message::Ptr::create();
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

void Composer::saveAsDraft()
{
    //TODO
    clear();
}

void Composer::clear()
{
    setSubject("");
    setBody("");
    setTo("");
    setCc("");
    setBcc("");
    setFromIndex(-1);
}