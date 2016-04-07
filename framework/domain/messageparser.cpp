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
#include "csshelper.h"

#include <QFile>
#include <QImage>
#include <QDebug>
#include <QQmlEngine>
#include <QTime>
#include <MimeTreeParser/ObjectTreeParser>
#include <MimeTreeParser/MessagePart>

PartModel::PartModel(QSharedPointer<MimeTreeParser::MessagePart> partTree, QMap<QByteArray, QUrl> embeddedPartMap) : mPartTree(partTree), mEmbeddedPartMap(embeddedPartMap)
{
}

QHash<int, QByteArray> PartModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Text] = "text";
    roles[IsHtml] = "isHtml";
    roles[IsEncrypted] = "isEncrypted";
    roles[IsAttachment] = "isAttachment";
    roles[HasContent] = "hasContent";
    roles[Type] = "type";
    return roles;
}

QModelIndex PartModel::index(int row, int column, const QModelIndex &parent) const
{
    // qDebug() << "index " << parent << row << column << mPartTree->messageParts().size();
    if (!parent.isValid()) {
        if (row < mPartTree->messageParts().size()) {
            auto part = mPartTree->messageParts().at(row);
            return createIndex(row, column, part.data());
        }
    } else {
        auto part = static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer());
        auto subPart = part->messageParts().at(row);
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
                for (const auto &cid : mEmbeddedPartMap.keys()) {
                    text.replace(QString("src=\"cid:%1\"").arg(QString(cid)), QString("src=\"%1\"").arg(mEmbeddedPartMap.value(cid).toString()));
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
        for (const auto &p : parentPart->messageParts()) {
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
        // qDebug() << "Row count " << mPartTree->messageParts().size();
        return mPartTree->messageParts().size();
    } else {
        auto part = static_cast<MimeTreeParser::MessagePart*>(parent.internalPointer());
        if (part) {
            return part->messageParts().size();
        }
    }
    return 0;
}

int PartModel::columnCount(const QModelIndex &parent) const
{
    // qDebug() << "Column count " << parent;
    return 1;
}


MessageParser::MessageParser(QObject *parent)
    : QObject(parent)
{

}

QString MessageParser::html() const
{
    return mHtml;
}

QVariant MessageParser::message() const
{
    return QVariant();
}

void MessageParser::setMessage(const QVariant &message)
{
    QTime time;
    time.start();
    const auto mailData = KMime::CRLFtoLF(message.toByteArray());
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(mailData);
    msg->parse();
    qWarning() << "parsed: " << time.elapsed();

    // render the mail
    StringHtmlWriter htmlWriter;
    QImage paintDevice;
    CSSHelper cssHelper(&paintDevice);
    //temporary files only have the lifetime of the nodehelper, so we keep it around until the mail changes.
    mNodeHelper = std::make_shared<MimeTreeParser::NodeHelper>();
    ObjectTreeSource source(&htmlWriter, &cssHelper);
    MimeTreeParser::ObjectTreeParser otp(&source, mNodeHelper.get());

    mPartTree = otp.parseToTree(msg.data()).dynamicCast<MimeTreeParser::MessagePart>();

    htmlWriter.begin(QString());
    htmlWriter.queue(cssHelper.htmlHead(false));

    if (mPartTree) {
        mPartTree->fix();
        mPartTree->copyContentFrom();
        mPartTree->html(false);
    }

    htmlWriter.queue(QStringLiteral("</body></html>"));
    htmlWriter.end();

    mEmbeddedPartMap = htmlWriter.embeddedParts();
    mHtml = htmlWriter.html();
    emit htmlChanged();
}

QAbstractItemModel *MessageParser::partTree() const
{
    qDebug() << "Getting partTree";
    qDebug() << "Row count " << mPartTree->messageParts().size();
    return new PartModel(mPartTree, mEmbeddedPartMap);
}

