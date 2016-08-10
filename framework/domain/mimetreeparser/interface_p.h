/*
    Copyright (c) 2016 Sandro Knauß <knauss@kolabsystems.com>

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

#include "interface.h"

#include <QSharedPointer>
#include <QMap>

namespace KMime
{
    class Message;
    typedef QSharedPointer<Message> MessagePtr;
}

namespace MimeTreeParser
{
    class MessagePart;
    class NodeHelper;
    typedef QSharedPointer<MessagePart> MessagePartPtr;
}

class ParserPrivate
{
public:
    ParserPrivate(Parser *parser);

    void setMessage(const QByteArray &mimeMessage);
    void createTree(const MimeTreeParser::MessagePartPtr& start, const Part::Ptr& tree);

    Part::Ptr mTree;
    Parser *q;

    MimeTreeParser::MessagePartPtr mPartTree;
    KMime::MessagePtr mMsg;
    std::shared_ptr<MimeTreeParser::NodeHelper> mNodeHelper;
    QString mHtml;
    QMap<QByteArray, QUrl> mEmbeddedPartMap;
};