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

#include <sink/mimetreeparser/objecttreeparser.h>
#include <QTime>
#include <sink/log.h>

#include "partmodel.h"
#include "attachmentmodel.h"
#include "modeltest.h"
#include "async.h"

class MessagePartPrivate
{
public:
    std::shared_ptr<MimeTreeParser::ObjectTreeParser> mParser;
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
    mRawContent = message.toString();
    asyncRun<std::shared_ptr<MimeTreeParser::ObjectTreeParser>>(this, [=] {
            QTime time;
            time.start();
            auto parser = std::make_shared<MimeTreeParser::ObjectTreeParser>();
            parser->parseObjectTree(message.toByteArray());
            SinkLog() << "Message parsing took: " << time.elapsed();
            parser->decryptParts();
            SinkLog() << "Message parsing and decryption/verification: " << time.elapsed();
            return parser;
        },
        [this](const std::shared_ptr<MimeTreeParser::ObjectTreeParser> &parser) {
            d->mParser = parser;
            emit htmlChanged();
        });
}

QString MessageParser::rawContent() const
{
    return mRawContent;
}

bool MessageParser::loaded() const
{
    return bool{d->mParser};
}

QString MessageParser::structureAsString() const
{
    if (!d->mParser) {
        return nullptr;
    }
    return d->mParser->structureAsString();
}

QAbstractItemModel *MessageParser::parts() const
{
    if (!d->mParser) {
        return nullptr;
    }
    const auto model = new PartModel(d->mParser);
    // new ModelTest(model, model);
    return model;
}

QAbstractItemModel *MessageParser::attachments() const
{
    if (!d->mParser) {
        return nullptr;
    }
    const auto model = new AttachmentModel(d->mParser);
    // new ModelTest(model, model);
    return model;
}
