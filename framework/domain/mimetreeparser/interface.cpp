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
#include "interface_p.h"

#include "stringhtmlwriter.h"
#include "objecttreesource.h"

#include <KMime/Content>
#include <MimeTreeParser/ObjectTreeParser>
#include <MimeTreeParser/MessagePart>
#include <MimeTreeParser/NodeHelper>

#include <QMimeDatabase>
#include <QMimeType>
#include <QTextCodec>
#include <QDebug>

class MailMimePrivate
{
public:
    MailMimePrivate(MailMime *p);

    MailMime *q;
    KMime::Content *mNode;
    std::shared_ptr<MailMime> parent;
};

MailMimePrivate::MailMimePrivate(MailMime* p)
    : q(p)
    , mNode(nullptr)
    , parent(nullptr)
{
}


MailMime::MailMime()
    : d(std::unique_ptr<MailMimePrivate>(new MailMimePrivate(this)))
{
}

bool MailMime::isFirstTextPart() const
{
    if (!d->mNode || !d->mNode->topLevel()) {
        return false;
    }
    return (d->mNode->topLevel()->textContent() == d->mNode);
}

bool MailMime::isFirstPart() const
{
   if (!d->mNode || !d->mNode->parent()) {
       return false;
   }
   return (d->mNode->parent()->contents().first() == d->mNode);
}

bool MailMime::isTopLevelPart() const
{
    if (!d->mNode) {
        return false;
    }
    return (d->mNode->topLevel() == d->mNode);
}

MailMime::Disposition MailMime::disposition() const
{
    if (!d->mNode) {
        return Invalid;
    }
    const auto cd = d->mNode->contentDisposition(false);
    if (!cd) {
        return Invalid;
    }
    switch (cd->disposition()){
        case KMime::Headers::CDinline:
            return Inline;
        case KMime::Headers::CDattachment:
            return Attachment;
        default:
            return Invalid;
    }
}

QString MailMime::filename() const
{
    if (!d->mNode) {
        return QString();
    }
    const auto cd = d->mNode->contentDisposition(false);
    if (!cd) {
        return QString();
    }
    return cd->filename();
}

QMimeType MailMime::mimetype() const
{
    if (!d->mNode) {
        return QMimeType();
    }

    const auto ct = d->mNode->contentType(false);
    if (!ct) {
        return QMimeType();
    }

    QMimeDatabase mimeDb;
    return mimeDb.mimeTypeForName(ct->mimeType());
}

MailMime::Ptr MailMime::parent() const
{
    if (!d->parent) {
        d->parent = std::shared_ptr<MailMime>(new MailMime());
        d->parent->d->mNode = d->mNode->parent();
    }
    return d->parent;
}

class PartPrivate
{
public:
    PartPrivate(Part *part);
    void appendSubPart(Part::Ptr subpart);

    QVector<Part::Ptr> subParts();

    Part *parent() const;

    const MailMime::Ptr &mailMime() const;
    void createMailMime(const MimeTreeParser::MimeMessagePart::Ptr &part);
    void createMailMime(const MimeTreeParser::TextMessagePart::Ptr &part);
    void createMailMime(const MimeTreeParser::AlternativeMessagePart::Ptr &part);
    void createMailMime(const MimeTreeParser::HtmlMessagePart::Ptr &part);

    void appendEncryption(const MimeTreeParser::EncryptedMessagePart::Ptr &part);
    void appendSignature(const MimeTreeParser::SignedMessagePart::Ptr &part);

    void setSignatures(const QVector<Signature::Ptr> &sigs);
    void setEncryptions(const QVector<Encryption::Ptr> &encs);

    const QVector<Encryption::Ptr> &encryptions() const;
    const QVector<Signature::Ptr> &signatures() const;
private:
    Part *q;
    Part *mParent;
    QVector<Part::Ptr> mSubParts;
    QVector<Encryption::Ptr> mEncryptions;
    QVector<Signature::Ptr> mSignatures;
    MailMime::Ptr mMailMime;
};

PartPrivate::PartPrivate(Part* part)
    : q(part)
    , mParent(Q_NULLPTR)
{

}

void PartPrivate::createMailMime(const MimeTreeParser::HtmlMessagePart::Ptr& part)
{
    mMailMime = MailMime::Ptr(new MailMime);
    mMailMime->d->mNode = part->mNode;
}

void PartPrivate::createMailMime(const MimeTreeParser::AlternativeMessagePart::Ptr& part)
{
    mMailMime = MailMime::Ptr(new MailMime);
    mMailMime->d->mNode = part->mNode;
}

void PartPrivate::createMailMime(const MimeTreeParser::TextMessagePart::Ptr& part)
{
    mMailMime = MailMime::Ptr(new MailMime);
    mMailMime->d->mNode = part->mNode;
}

void PartPrivate::createMailMime(const MimeTreeParser::MimeMessagePart::Ptr& part)
{
    mMailMime = MailMime::Ptr(new MailMime);
    mMailMime->d->mNode = part->mNode;
}

void PartPrivate::appendSubPart(Part::Ptr subpart)
{
    subpart->d->mParent = q;
    mSubParts.append(subpart);
}

void PartPrivate::appendEncryption(const MimeTreeParser::EncryptedMessagePart::Ptr& part)
{
    mEncryptions.append(Encryption::Ptr(new Encryption));
}

void PartPrivate::setEncryptions(const QVector< Encryption::Ptr >& encs)
{
    mEncryptions = encs;
}

void PartPrivate::appendSignature(const MimeTreeParser::SignedMessagePart::Ptr& part)
{
    mSignatures.append(Signature::Ptr(new Signature));
}


void PartPrivate::setSignatures(const QVector< Signature::Ptr >& sigs)
{
    mSignatures = sigs;
}

Part *PartPrivate::parent() const
{
    return mParent;
}

QVector< Part::Ptr > PartPrivate::subParts()
{
    return mSubParts;
}

const MailMime::Ptr& PartPrivate::mailMime() const
{
    return mMailMime;
}

const QVector< Encryption::Ptr >& PartPrivate::encryptions() const
{
    return mEncryptions;
}

const QVector< Signature::Ptr >& PartPrivate::signatures() const
{
    return mSignatures;
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

QVector<QByteArray> Part::availableContents() const
{
    return QVector<QByteArray>();
}

QVector<Content::Ptr> Part::content() const
{
    return content(availableContents().first());
}

QVector<Content::Ptr> Part::content(const QByteArray& ct) const
{
    return QVector<Content::Ptr>();
}

QVector<Encryption::Ptr> Part::encryptions() const
{
    auto ret = d->encryptions();
    auto parent = d->parent();
    if (parent) {
        ret.append(parent->encryptions());
    }
    return ret;
}

QVector<Signature::Ptr> Part::signatures() const
{
    auto ret = d->signatures();
    auto parent = d->parent();
    if (parent) {
        ret.append(parent->signatures());
    }
    return ret;
}

MailMime::Ptr Part::mailMime() const
{
    return d->mailMime();
}

Part *Part::parent() const
{
    return d->parent();
}

class ContentPrivate
{
public:
    QByteArray mContent;
    QByteArray mCodec;
    Part *mParent;
    Content *q;
    MailMime::Ptr mMailMime;
    QVector<Encryption::Ptr> mEncryptions;
    QVector<Signature::Ptr> mSignatures;
    void appendSignature(const MimeTreeParser::SignedMessagePart::Ptr &sig);
    void appendEncryption(const MimeTreeParser::EncryptedMessagePart::Ptr &enc);
};

void ContentPrivate::appendEncryption(const MimeTreeParser::EncryptedMessagePart::Ptr& enc)
{
    mEncryptions.append(Encryption::Ptr(new Encryption));
}

void ContentPrivate::appendSignature(const MimeTreeParser::SignedMessagePart::Ptr& sig)
{
    mSignatures.append(Signature::Ptr(new Signature));
}


Content::Content(const QByteArray& content, Part *parent)
    : d(std::unique_ptr<ContentPrivate>(new ContentPrivate))
{
    d->q = this;
    d->mContent = content;
    d->mCodec = "utf-8";
    d->mParent = parent;
}

Content::Content(ContentPrivate* d_ptr)
    : d(std::unique_ptr<ContentPrivate>(d_ptr))
{
    d->q = this;
}

Content::~Content()
{
}

QVector<Encryption::Ptr> Content::encryptions() const
{
    auto ret = d->mEncryptions;
    if (d->mParent) {
        ret.append(d->mParent->encryptions());
    }
    return ret;
}

QVector<Signature::Ptr> Content::signatures() const
{
    auto ret = d->mSignatures;
    if (d->mParent) {
        ret.append(d->mParent->signatures());
    }
    return ret;
}

QByteArray Content::content() const
{
    return d->mContent;
}

QByteArray Content::charset() const
{
    return d->mCodec;
}

QString Content::encodedContent() const
{
    return encodedContent(charset());
}

QString Content::encodedContent(const QByteArray &charset) const
{
    QTextCodec *codec = QTextCodec::codecForName(charset);
    return codec->toUnicode(content());
}

QByteArray Content::type() const
{
    return "Content";
}

MailMime::Ptr Content::mailMime() const
{
    if (d->mMailMime) {
        return d->mMailMime;
    } else {
        return d->mParent->mailMime();
    }
}

Part *Content::parent() const
{
    return d->mParent;
}

HtmlContent::HtmlContent(const QByteArray& content, Part* parent)
    : Content(content, parent)
{

}

QByteArray HtmlContent::type() const
{
    return "HtmlContent";
}

PlainTextContent::PlainTextContent(const QByteArray& content, Part* parent)
    : Content(content, parent)
{

}

PlainTextContent::PlainTextContent(ContentPrivate* d_ptr)
    : Content(d_ptr)
{

}

HtmlContent::HtmlContent(ContentPrivate* d_ptr)
    : Content(d_ptr)
{

}


QByteArray PlainTextContent::type() const
{
    return "PlainTextContent";
}

class AlternativePartPrivate
{
public:
    void fillFrom(MimeTreeParser::AlternativeMessagePart::Ptr part);

    QVector<Content::Ptr> content(const QByteArray &ct) const;

    AlternativePart *q;

    QVector<QByteArray> types() const;

private:
    QMap<QByteArray, QVector<Content::Ptr>> mContent;
    QVector<QByteArray> mTypes;
};

void AlternativePartPrivate::fillFrom(MimeTreeParser::AlternativeMessagePart::Ptr part)
{
    mTypes = QVector<QByteArray>() << "html" << "plaintext";

    Content::Ptr content = std::make_shared<HtmlContent>(part->htmlContent().toLocal8Bit(), q);
    mContent["html"].append(content);
    content = std::make_shared<PlainTextContent>(part->plaintextContent().toLocal8Bit(), q);
    mContent["plaintext"].append(content);
    q->reachParentD()->createMailMime(part);
}

QVector<QByteArray> AlternativePartPrivate::types() const
{
    return mTypes;
}

QVector<Content::Ptr> AlternativePartPrivate::content(const QByteArray& ct) const
{
    return mContent[ct];
}

AlternativePart::AlternativePart()
    : d(std::unique_ptr<AlternativePartPrivate>(new AlternativePartPrivate))
{
    d->q = this;
}

AlternativePart::~AlternativePart()
{

}

QByteArray AlternativePart::type() const
{
    return "AlternativePart";
}

QVector<QByteArray> AlternativePart::availableContents() const
{
    return d->types();
}

QVector<Content::Ptr> AlternativePart::content(const QByteArray& ct) const
{
    return d->content(ct);
}

PartPrivate* AlternativePart::reachParentD() const
{
    return Part::d.get();
}

class SinglePartPrivate
{
public:
    void fillFrom(MimeTreeParser::TextMessagePart::Ptr part);
    void fillFrom(MimeTreeParser::HtmlMessagePart::Ptr part);
    void fillFrom(MimeTreeParser::AttachmentMessagePart::Ptr part);
    SinglePart *q;

    QVector<Content::Ptr> mContent;
    QByteArray mType;
};

void SinglePartPrivate::fillFrom(MimeTreeParser::TextMessagePart::Ptr part)
{
    mType = "plaintext";
    mContent.clear();
    foreach (const auto &mp, part->subParts()) {
        auto d_ptr = new ContentPrivate;
        d_ptr->mContent = mp->text().toLocal8Bit();
        d_ptr->mParent = q;
        d_ptr->mCodec = "utf-8";
        const auto enc = mp.dynamicCast<MimeTreeParser::EncryptedMessagePart>();
        auto sig = mp.dynamicCast<MimeTreeParser::SignedMessagePart>();
        if (enc) {
           d_ptr->appendEncryption(enc);
           const auto s = enc->subParts();
           if (s.size() == 1) {
               sig = s[0].dynamicCast<MimeTreeParser::SignedMessagePart>();
           }
        }
        if (sig) {
           d_ptr->appendSignature(sig);
        }
        mContent.append(std::make_shared<PlainTextContent>(d_ptr));
        q->reachParentD()->createMailMime(part);
    }
}

void SinglePartPrivate::fillFrom(MimeTreeParser::HtmlMessagePart::Ptr part)
{
    mType = "html";
    mContent.clear();
    mContent.append(std::make_shared<HtmlContent>(part->text().toLocal8Bit(), q));
    q->reachParentD()->createMailMime(part);
}

void SinglePartPrivate::fillFrom(MimeTreeParser::AttachmentMessagePart::Ptr part)
{
   q->reachParentD()->createMailMime(part.staticCast<MimeTreeParser::TextMessagePart>());
   mType = q->mailMime()->mimetype().name().toUtf8();
   mContent.clear();
   mContent.append(std::make_shared<Content>(part->text().toLocal8Bit(), q));
}

SinglePart::SinglePart()
    : d(std::unique_ptr<SinglePartPrivate>(new SinglePartPrivate))
{
    d->q = this;
}

SinglePart::~SinglePart()
{

}

QVector<QByteArray> SinglePart::availableContents() const
{
    return QVector<QByteArray>() << d->mType;
}

QVector< Content::Ptr > SinglePart::content(const QByteArray &ct) const
{
    if (ct == d->mType) {
        return d->mContent;
    }
    return QVector<Content::Ptr>();
}

QByteArray SinglePart::type() const
{
    return "SinglePart";
}

PartPrivate* SinglePart::reachParentD() const
{
    return Part::d.get();
}

class SignaturePrivate
{
public:
    Signature *q;
};

Signature::Signature()
    :d(std::unique_ptr<SignaturePrivate>(new SignaturePrivate))
{
    d->q = this;
}


Signature::Signature(SignaturePrivate *d_ptr)
    :d(std::unique_ptr<SignaturePrivate>(d_ptr))
{
    d->q = this;
}

Signature::~Signature()
{

}


class EncryptionPrivate
{
public:
    Encryption *q;
};

Encryption::Encryption(EncryptionPrivate *d_ptr)
    :d(std::unique_ptr<EncryptionPrivate>(d_ptr))
{
    d->q = this;
}

Encryption::Encryption()
    :d(std::unique_ptr<EncryptionPrivate>(new EncryptionPrivate))
{
    d->q = this;
}

Encryption::~Encryption()
{

}

ParserPrivate::ParserPrivate(Parser* parser)
    : q(parser)
    , mNodeHelper(std::make_shared<MimeTreeParser::NodeHelper>())
{

}

void ParserPrivate::setMessage(const QByteArray& mimeMessage)
{
    const auto mailData = KMime::CRLFtoLF(mimeMessage);
    mMsg = KMime::Message::Ptr(new KMime::Message);
    mMsg->setContent(mailData);
    mMsg->parse();

    // render the mail
    StringHtmlWriter htmlWriter;
    ObjectTreeSource source(&htmlWriter);
    MimeTreeParser::ObjectTreeParser otp(&source, mNodeHelper.get());

    otp.parseObjectTree(mMsg.data());
    mPartTree = otp.parsedPart().dynamicCast<MimeTreeParser::MessagePart>();

    mEmbeddedPartMap = htmlWriter.embeddedParts();
    mHtml = htmlWriter.html();

    mTree = std::make_shared<Part>();
    createTree(mPartTree, mTree);
}


void ParserPrivate::createTree(const MimeTreeParser::MessagePart::Ptr &start, const Part::Ptr &tree)
{
    foreach (const auto &mp, start->subParts()) {
        const auto m = mp.dynamicCast<MimeTreeParser::MessagePart>();
        const auto text = mp.dynamicCast<MimeTreeParser::TextMessagePart>();
        const auto alternative = mp.dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        const auto html = mp.dynamicCast<MimeTreeParser::HtmlMessagePart>();
        const auto attachment = mp.dynamicCast<MimeTreeParser::AttachmentMessagePart>();
         if (attachment) {
            auto part = std::make_shared<SinglePart>();
            part->d->fillFrom(attachment);
            tree->d->appendSubPart(part);
         } else if (text) {
            auto part = std::make_shared<SinglePart>();
            part->d->fillFrom(text);
            tree->d->appendSubPart(part);
        } else if (alternative) {
            auto part = std::make_shared<AlternativePart>();
            part->d->fillFrom(alternative);
            tree->d->appendSubPart(part);
        } else if (html) {
            auto part = std::make_shared<SinglePart>();
            part->d->fillFrom(html);
            tree->d->appendSubPart(part);
        } else {
            const auto enc = mp.dynamicCast<MimeTreeParser::EncryptedMessagePart>();
            const auto sig = mp.dynamicCast<MimeTreeParser::SignedMessagePart>();
            if (enc || sig) {
                auto subTree =  std::make_shared<Part>();
                if (enc) {
                    subTree->d->appendEncryption(enc);
                }
                if (sig) {
                    subTree->d->appendSignature(sig);
                }
                createTree(m, subTree);
                foreach(const auto &p, subTree->subParts()) {
                    tree->d->appendSubPart(p);
                    if (enc) {
                        p->d->setEncryptions(subTree->d->encryptions());
                    }
                    if (sig) {
                        p->d->setSignatures(subTree->d->signatures());
                    }
                }
            } else {
                createTree(m, tree);
            }
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

QUrl Parser::getPart(const QByteArray &cid)
{
    return d->mEmbeddedPartMap.value(cid);
}

QVector<Part::Ptr> Parser::collectContentParts() const
{
    return collect(d->mTree, [](const Part::Ptr &p){return p->type() != "EncapsulatedPart";},
                             [](const Content::Ptr &content){
                                    const auto mime = content->mailMime();

                                    if (!mime) {
                                        return true;
                                    }

                                    if (mime->isFirstTextPart()) {
                                        return true;
                                    }

                                    {
                                        auto _mime = content->parent()->mailMime();
                                        while (_mime) {
                                            if (_mime && (_mime->isTopLevelPart() || _mime->isFirstTextPart())) {
                                                return true;
                                            }
                                            if (_mime->isFirstPart()) {
                                                _mime = _mime->parent();
                                            } else {
                                                break;
                                            }
                                        }
                                    }
                                    const auto cd = mime->disposition();
                                    if (cd && cd == MailMime::Inline) {
                                        // explict "inline" disposition:
                                        return true;
                                    }
                                    if (cd && cd == MailMime::Attachment) {
                                        // explicit "attachment" disposition:
                                        return false;
                                    }

                                    const auto ct = mime->mimetype();
                                    if (ct.name().trimmed().toLower() == "text" && ct.name().trimmed().isEmpty() &&
                                        (!mime || mime->filename().trimmed().isEmpty())) {
                                        // text/* w/o filename parameter:
                                        return true;
                                    }
                                    return false;
                             });
}


QVector<Part::Ptr> Parser::collectAttachmentParts() const
{
    return collect(d->mTree, [](const Part::Ptr &p){return p->type() != "EncapsulatedPart";},
                             [](const Content::Ptr &content){
                                    const auto mime = content->mailMime();

                                    if (!mime) {
                                        return false;
                                    }

                                    if (mime->isFirstTextPart()) {
                                        return false;
                                    }

                                    {
                                        const auto parent = content->parent();
                                        if (parent) {
                                            const auto _mime = parent->mailMime();
                                            if (_mime && (_mime->isTopLevelPart() || _mime->isFirstTextPart())) {
                                                return false;
                                            }
                                        }
                                    }
                                    const auto cd = mime->disposition();
                                    if (cd && cd == MailMime::Inline) {
                                        // explict "inline" disposition:
                                        return false;
                                    }
                                    if (cd && cd == MailMime::Attachment) {
                                        // explicit "attachment" disposition:
                                        return true;
                                    }

                                    const auto ct = mime->mimetype();
                                    if (ct.name().trimmed().toLower() == "text" && ct.name().trimmed().isEmpty() &&
                                        (!mime || mime->filename().trimmed().isEmpty())) {
                                        // text/* w/o filename parameter:
                                        return false;
                                    }
                                    return true;
                             });
}
QVector<Part::Ptr> Parser::collect(const Part::Ptr &start, std::function<bool(const Part::Ptr &)> select, std::function<bool(const Content::Ptr &)> filter) const
{
    QVector<Part::Ptr> ret;
    foreach (const auto &part, start->subParts()) {
        QVector<QByteArray> contents;
        foreach(const auto &ct, part->availableContents()) {
            foreach(const auto &content, part->content(ct)) {
                if (filter(content)) {
                    contents.append(ct);
                    break;
                }
            }
        }
        if (!contents.isEmpty()) {
            ret.append(part);
        }
        if (select(part)){
            ret += collect(part, select, filter);
        }
    }
    return ret;
}
