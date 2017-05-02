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
#include "messageparser.h"

#include "modeltest.h"
#include "mimetreeparser/interface.h"

#include <QDebug>

class MessagePartPrivate
{
public:
    std::shared_ptr<Parser> mParser;
};

MessageParser::MessageParser(QObject *parent)
    : QObject(parent)
    , d(std::unique_ptr<MessagePartPrivate>(new MessagePartPrivate))
{

}

MessageParser::~MessageParser()
{

}

QVariant MessageParser::message() const
{
    return QVariant();
}

void MessageParser::setMessage(const QVariant &message)
{
    d->mParser = std::shared_ptr<Parser>(new Parser(message.toByteArray()));
    emit htmlChanged();
}

QAbstractItemModel *MessageParser::newTree() const
{
    const auto model = new NewModel(d->mParser);
    // new ModelTest(model, model);
    return model;
}

QAbstractItemModel *MessageParser::attachments() const
{
    const auto model = new AttachmentModel(d->mParser);
    // new ModelTest(model, model);
    return model;
}
