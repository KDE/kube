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

#include <QByteArray>
#include <KMime/Message>
#include <functional>
#include <mailcrypto.h>

struct Attachment {
    QString name;
    QString filename;
    QByteArray mimeType;
    bool isInline;
    QByteArray data;
};

namespace MailTemplates
{
    void reply(const KMime::Message::Ptr &origMsg, const std::function<void(const KMime::Message::Ptr &result)> &callback, const KMime::Types::AddrSpecList &me = {});
    void forward(const KMime::Message::Ptr &origMsg, const std::function<void(const KMime::Message::Ptr &result)> &callback);
    QString plaintextContent(const KMime::Message::Ptr &origMsg);
    QString body(const KMime::Message::Ptr &msg, bool &isHtml);
    KMime::Message::Ptr createMessage(KMime::Message::Ptr existingMessage, const QStringList &to, const QStringList &cc, const QStringList &bcc, const KMime::Types::Mailbox &from, const QString &subject, const QString &body, bool htmlBody, const QList<Attachment> &attachments, const std::vector<MailCrypto::Key> &signingKeys = {}, const std::vector<MailCrypto::Key> &encryptionKeys = {}, const MailCrypto::Key &attachedKey = {});
};
