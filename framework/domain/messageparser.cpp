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
#include "stringhtmlwriter.h"
#include "objecttreesource.h"

#include "mimetreeparser/interface.h"

#include <QRegExp>

#include <QFile>
#include <QImage>
#include <QDebug>
#include <QTime>
#include <QUrl>
#include <MimeTreeParser/ObjectTreeParser>
#include <MimeTreeParser/MessagePart>


class MessagePartPrivate
{
public:
    QSharedPointer<MimeTreeParser::MessagePart> mPartTree;
    QString mHtml;
    QMap<QByteArray, QUrl> mEmbeddedPartMap;
    std::shared_ptr<MimeTreeParser::NodeHelper> mNodeHelper;
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

QString MessageParser::html() const
{
    return d->mHtml;
}

QVariant MessageParser::message() const
{
    return QVariant();
}

void MessageParser::setMessage(const QVariant &message)
{
    QTime time;
    time.start();
    d->mParser = std::shared_ptr<Parser>(new Parser(message.toByteArray()));

    const auto mailData = KMime::CRLFtoLF(message.toByteArray());
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(mailData);
    msg->parse();
    qWarning() << "parsed: " << time.elapsed();

    // render the mail
    StringHtmlWriter htmlWriter;
    //temporary files only have the lifetime of the nodehelper, so we keep it around until the mail changes.
    d->mNodeHelper = std::make_shared<MimeTreeParser::NodeHelper>();
    ObjectTreeSource source(&htmlWriter);
    MimeTreeParser::ObjectTreeParser otp(&source, d->mNodeHelper.get());

    otp.parseObjectTree(msg.data());
    d->mPartTree = otp.parsedPart().dynamicCast<MimeTreeParser::MessagePart>();

    d->mEmbeddedPartMap = htmlWriter.embeddedParts();
    d->mHtml = htmlWriter.html();
    emit htmlChanged();
}

QAbstractItemModel *MessageParser::partTree() const
{
    return new PartModel(d->mPartTree, d->mParser);
}

QAbstractItemModel *MessageParser::newTree() const
{
    const auto model = new NewModel(d->mParser);
    new ModelTest(model, model);
    return model;
}

QAbstractItemModel *MessageParser::attachments() const
{
    const auto model = new AttachmentModel(d->mParser);
    new ModelTest(model, model);
    return model;
}
