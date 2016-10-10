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

PartModel::PartModel(QSharedPointer<MimeTreeParser::MessagePart> partTree, std::shared_ptr<Parser> parser)
    : mPartTree(partTree)
    , mParser(parser)
{
}

QHash<int, QByteArray> PartModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Text] = "text";
    roles[IsHtml] = "isHtml";
    roles[IsHidden] = "isHidden";
    roles[IsEncrypted] = "isEncrypted";
    roles[IsAttachment] = "isAttachment";
    roles[HasContent] = "hasContent";
    roles[Type] = "type";
    roles[IsHidden] = "isHidden";
    return roles;
}

QModelIndex PartModel::index(int row, int column, const QModelIndex &parent) const
{
    // qDebug() << "index " << parent << row << column << mPartTree->subParts().size();
    if (!parent.isValid()) {
        if (row < mPartTree->subParts().size()) {
            auto part = mPartTree->subParts().at(row);
            return createIndex(row, column, part.data());
        }
    } else {
        auto part = static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer());
        auto subPart = part->subParts().at(row);
        return createIndex(row, column, subPart.data());
    }
    return QModelIndex();
}

QVariant PartModel::data(const QModelIndex &index, int role) const
{
    // qDebug() << "Getting data for index";
    if (index.isValid()) {
        auto part = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
        switch (role) {
            case Text: {
                // qDebug() << "Getting text: " << part->property("text").toString();
                // FIXME: we should have a list per part, and not one for all parts.
                auto text = part->property("htmlContent").toString();
                auto rx = QRegExp("src=(\"|')cid:([^\1]*)\1");
                int pos  = 0;
                while ((pos = rx.indexIn(text, pos)) != -1) {
                    auto repl = mParser->getPart(rx.cap(2).toUtf8());
                    if (repl.isValid()) {
                        text.replace(rx.cap(0), QString("src=\"%1\"").arg(repl.toString()));
                    }
                    pos += rx.matchedLength();
                }
                return text;
            }
            case IsAttachment:
                return part->property("attachment").toBool();
            case IsEncrypted:
                return part->property("isEncrypted").toBool();
            case IsHtml:
                return part->property("isHtml").toBool();
            case HasContent:
                return !part->property("htmlContent").toString().isEmpty();
            case Type:
                return part->metaObject()->className();
            case IsHidden:
                return false;
                //return part->property("isHidden").toBool();

        }
    }
    return QVariant();
}

QModelIndex PartModel::parent(const QModelIndex &index) const
{
    // qDebug() << "parent " << index;
    if (index.isValid()) {
        auto part = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
        auto parentPart = static_cast<MimeTreeParser::MessagePart*>(part->parentPart());
        auto row = 0;//get the parents parent to find the index
        if (!parentPart) {
            parentPart = mPartTree.data();
        }
        int i = 0;
        for (const auto &p : parentPart->subParts()) {
            if (p.data() == part) {
                row = i;
                break;
            }
            i++;
        }
        return createIndex(row, index.column(), parentPart);
    }
    return QModelIndex();
}

int PartModel::rowCount(const QModelIndex &parent) const
{
    // qDebug() << "Row count " << parent;
    if (!parent.isValid()) {
        // qDebug() << "Row count " << mPartTree->subParts().size();
        return mPartTree->subParts().size();
    } else {
        auto part = static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer());
        if (part) {
            return part->subParts().size();
        }
    }
    return 0;
}

int PartModel::columnCount(const QModelIndex &parent) const
{
    // qDebug() << "Column count " << parent;
    return 1;
}

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

