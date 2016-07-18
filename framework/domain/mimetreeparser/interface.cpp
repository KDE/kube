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

#include "interface.h"

#include "stringhtmlwriter.h"
#include "objecttreesource.h"

#include <KMime/Content>
#include <MimeTreeParser/ObjectTreeParser>
#include <MimeTreeParser/MessagePart>
#include <MimeTreeParser/NodeHelper>

#include <QDebug>
#include <QImage>

class PartPrivate
{
public:
    PartPrivate(Part *part);
    void appendSubPart(Part::Ptr subpart);

    QVector<Part::Ptr> subParts();

    const std::weak_ptr<Part> &parent() const;
private:
    Part *q;
    std::weak_ptr<Part> mParent;
    QVector<Part::Ptr> mSubParts;
};

PartPrivate::PartPrivate(Part* part)
    :q(part)
{

}

void PartPrivate::appendSubPart(Part::Ptr subpart)
{
    subpart->d->mParent = Part::Ptr(q);
    mSubParts.append(subpart);
}

const std::weak_ptr<Part> &PartPrivate::parent() const
{
    return mParent;
}

QVector< Part::Ptr > PartPrivate::subParts()
{
    return mSubParts;
}

Part::Part()
    : d(std::unique_ptr<PartPrivate>(new PartPrivate(this)))
{

}

bool Part::hasSubParts() const
{
    return !subParts().isEmpty();
}

QVector<Part::Ptr> Part::subParts() const
{
    return d->subParts();
}

QByteArray Part::type() const
{
    return "Part";
}

QVector<Encryption> Part::encryptions() const
{
    auto parent = d->parent().lock();
    if (parent) {
        return parent->encryptions();
    } else {
        return QVector<Encryption>();
    }
}

QVector<Signature> Part::signatures() const
{
    auto parent = d->parent().lock();
    if (parent) {
        return parent->signatures();
    } else {
        return QVector<Signature>();
    }
}

class ContentPrivate
{
public:
    QByteArray mContent;
    QByteArray mCodec;
    Part *mParent;
    Content *q;
};

Content::Content(const QByteArray& content, ContentPart *parent)
    : d(std::unique_ptr<ContentPrivate>(new ContentPrivate))
{
    d->q = this;
    d->mContent = content;
    d->mCodec = "utf-8";
    d->mParent = parent;
}

Content::~Content()
{
}

QVector< Encryption > Content::encryptions() const
{
    if (d->mParent) {
        return d->mParent->encryptions();
    }
    return QVector<Encryption>();
}

QVector< Signature > Content::signatures() const
{
    if (d->mParent) {
        return d->mParent->signatures();
    }
    return QVector<Signature>();
}

class ContentPartPrivate
{
public:
    void fillFrom(MimeTreeParser::TextMessagePart::Ptr part);
    void fillFrom(MimeTreeParser::HtmlMessagePart::Ptr part);
    void fillFrom(MimeTreeParser::AlternativeMessagePart::Ptr part);

    QVector<Content::Ptr> contents() const;

    ContentPart *q;
    
private:
    QVector<Content::Ptr> mContents;
    ContentPart::Types mTypes;
};

void ContentPartPrivate::fillFrom(MimeTreeParser::TextMessagePart::Ptr part)
{
    mTypes = ContentPart::PlainText;
    foreach (const auto &mp, part->subParts()) {
        auto content = std::make_shared<Content>(mp->text().toLocal8Bit(), q);
        mContents.append(content);
    }
}

void ContentPartPrivate::fillFrom(MimeTreeParser::HtmlMessagePart::Ptr part)
{
    mTypes = ContentPart::Html;
}

void ContentPartPrivate::fillFrom(MimeTreeParser::AlternativeMessagePart::Ptr part)
{
    mTypes = ContentPart::Html | ContentPart::PlainText;
}

ContentPart::ContentPart()
    : d(std::unique_ptr<ContentPartPrivate>(new ContentPartPrivate))
{
    d->q = this;
}

ContentPart::~ContentPart()
{

}

QByteArray ContentPart::type() const
{
    return "ContentPart";
}

class MimePartPrivate
{
public:
    void fillFrom(MimeTreeParser::MessagePart::Ptr part);
};

QByteArray MimePart::type() const
{
    return "MimePart";
}

class AttachmentPartPrivate
{
public:
    void fillFrom(MimeTreeParser::AttachmentMessagePart::Ptr part);
};

void AttachmentPartPrivate::fillFrom(MimeTreeParser::AttachmentMessagePart::Ptr part)
{

}

QByteArray AttachmentPart::type() const
{
    return "AttachmentPart";
}

class ParserPrivate
{
public:
    ParserPrivate(Parser *parser);

    void setMessage(const QByteArray &mimeMessage);
    void createTree(MimeTreeParser::MessagePart::Ptr start, Part::Ptr tree);

    Part::Ptr mTree;
private:
    Parser *q;

    MimeTreeParser::MessagePart::Ptr mPartTree;
    std::shared_ptr<MimeTreeParser::NodeHelper> mNodeHelper;
    QString mHtml;
    QMap<QByteArray, QUrl> mEmbeddedPartMap;
};

ParserPrivate::ParserPrivate(Parser* parser)
    : q(parser)
    , mNodeHelper(std::make_shared<MimeTreeParser::NodeHelper>())
{

}

void ParserPrivate::setMessage(const QByteArray& mimeMessage)
{
    const auto mailData = KMime::CRLFtoLF(mimeMessage);
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(mailData);
    msg->parse();

    // render the mail
    StringHtmlWriter htmlWriter;
    QImage paintDevice;
    ObjectTreeSource source(&htmlWriter);
    MimeTreeParser::ObjectTreeParser otp(&source, mNodeHelper.get());

    otp.parseObjectTree(msg.data());
    mPartTree = otp.parsedPart().dynamicCast<MimeTreeParser::MessagePart>();

    mEmbeddedPartMap = htmlWriter.embeddedParts();
    mHtml = htmlWriter.html();

    mTree = std::make_shared<Part>();
    createTree(mPartTree, mTree);
}


void ParserPrivate::createTree(MimeTreeParser::MessagePart::Ptr start, Part::Ptr tree)
{

    foreach (const auto &mp, start->subParts()) {
        const auto m = mp.dynamicCast<MimeTreeParser::MessagePart>();
        const auto text = mp.dynamicCast<MimeTreeParser::TextMessagePart>();
        const auto alternative = mp.dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        const auto html = mp.dynamicCast<MimeTreeParser::HtmlMessagePart>();
        const auto attachment = mp.dynamicCast<MimeTreeParser::AttachmentMessagePart>();
        if (text) {
            auto part = std::make_shared<ContentPart>();
            part->d->fillFrom(text);
            mTree->d->appendSubPart(part);
        } else if (alternative) {
            auto part = std::make_shared<ContentPart>();
            part->d->fillFrom(alternative);
            mTree->d->appendSubPart(part);
        } else if (html) {
            auto part = std::make_shared<ContentPart>();
            part->d->fillFrom(html);
            mTree->d->appendSubPart(part);
        } else if (attachment) {
            auto part = std::make_shared<AttachmentPart>();
            part->d->fillFrom(attachment);
            mTree->d->appendSubPart(part);
        } else {
            createTree(m, tree);
        }
    }
}

Parser::Parser(const QByteArray& mimeMessage)
    :d(std::unique_ptr<ParserPrivate>(new ParserPrivate(this)))
{
    d->setMessage(mimeMessage);
}

Parser::~Parser()
{
}

ContentPart::Ptr Parser::collectContentPart(const Part::Ptr &start) const
{
    foreach (const auto &part, start->subParts()) {
        if (part->type() == "ContentPart") {
            return std::dynamic_pointer_cast<ContentPart>(part);
        } else {
            auto ret = collectContentPart(part);
            if (ret) {
                return ret;
            }
        }
    }
    return ContentPart::Ptr();
}

ContentPart::Ptr Parser::collectContentPart() const
{
    return collectContentPart(d->mTree);
}
