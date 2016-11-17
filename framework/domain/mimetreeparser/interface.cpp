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

#include <QGpgME/KeyListJob>
#include <QGpgME/Protocol>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

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

QByteArray MailMime::cid() const
{
    if (!d->mNode || !d->mNode->contentID()) {
        return QByteArray();
    }
    return d->mNode->contentID()->identifier();
}

QByteArray MailMime::charset() const
{
    if(!d->mNode || !d->mNode->contentType(false)) {
        return QByteArray();
    }
    if (d->mNode->contentType(false)) {
        return d->mNode->contentType(false)->charset();
    }
    return d->mNode->defaultCharset();
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

QByteArray MailMime::decodedContent() const
{
    if (!d->mNode) {
        return QByteArray();
    }
    return d->mNode->decodedContent();
}

class KeyPrivate
{
public:
    Key *q;
    GpgME::Key mKey;
    QByteArray mKeyID;
};

Key::Key()
    :d(std::unique_ptr<KeyPrivate>(new KeyPrivate))
{
    d->q = this;
}


Key::Key(KeyPrivate *d_ptr)
    :d(std::unique_ptr<KeyPrivate>(d_ptr))
{
    d->q = this;
}

Key::~Key()
{

}

QString Key::keyid() const
{
    if (!d->mKey.isNull()) {
        return d->mKey.keyID();
    }

    return d->mKeyID;
}

QString Key::name() const
{
    //FIXME: is this the correct way to get the primary UID?
    if (!d->mKey.isNull()) {
        return d->mKey.userID(0).name();
    }

    return QString();
}

QString Key::email() const
{
    if (!d->mKey.isNull()) {
        return d->mKey.userID(0).email();
    }
    return QString();
}

QString Key::comment() const
{
    if (!d->mKey.isNull()) {
        return d->mKey.userID(0).comment();
    }
    return QString();
}

class SignaturePrivate
{
public:
    Signature *q;
    GpgME::Signature mSignature;
    Key::Ptr mKey;
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

QDateTime Signature::creationDateTime() const
{
    QDateTime dt;
    dt.setTime_t(d->mSignature.creationTime());
    return dt;
}

QDateTime Signature::expirationDateTime() const
{
    QDateTime dt;
    dt.setTime_t(d->mSignature.expirationTime());
    return dt;
}

bool Signature::neverExpires() const
{
    return d->mSignature.neverExpires();
}

Key::Ptr Signature::key() const
{
    return d->mKey;
}

class EncryptionPrivate
{
public:
    Encryption *q;
    std::vector<Key::Ptr> mRecipients;
    Encryption::ErrorType mErrorType;
    QString mErrorString;
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
    d->mErrorType = Encryption::NoError;
}

Encryption::~Encryption()
{

}

std::vector<Key::Ptr> Encryption::recipients() const
{
    return d->mRecipients;
}

QString Encryption::errorString()
{
    return d->mErrorString;
}

Encryption::ErrorType Encryption::errorType()
{
    return d->mErrorType;
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
    void createMailMime(const MimeTreeParser::EncryptedMessagePart::Ptr &part);

    static Encryption::Ptr createEncryption(const MimeTreeParser::EncryptedMessagePart::Ptr& part);
    void appendEncryption(const MimeTreeParser::EncryptedMessagePart::Ptr &part);
    static QVector<Signature::Ptr> createSignature(const MimeTreeParser::SignedMessagePart::Ptr& part);
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

void PartPrivate::createMailMime(const MimeTreeParser::EncryptedMessagePart::Ptr& part)
{
    mMailMime = MailMime::Ptr(new MailMime);
    mMailMime->d->mNode = part->mNode;
}

void PartPrivate::appendSubPart(Part::Ptr subpart)
{
    subpart->d->mParent = q;
    mSubParts.append(subpart);
}

Encryption::Ptr PartPrivate::createEncryption(const MimeTreeParser::EncryptedMessagePart::Ptr& part)
{
    QGpgME::KeyListJob *job = part->mCryptoProto->keyListJob(false);    // local, no sigs
    if (!job) {
        qWarning() << "The Crypto backend does not support listing keys. ";
        return Encryption::Ptr();
    }

    auto encpriv = new EncryptionPrivate();
    if (part->passphraseError()) {
        encpriv->mErrorType = Encryption::PassphraseError;
        encpriv->mErrorString = part->mMetaData.errorText;
    } else if (part->isEncrypted() && !part->isDecryptable()) {
        encpriv->mErrorType = Encryption::KeyMissing;
        encpriv->mErrorString = part->mMetaData.errorText;
    } else if (!part->isEncrypted() && !part->isDecryptable()) {
        encpriv->mErrorType = Encryption::UnknownError;
        encpriv->mErrorString = part->mMetaData.errorText;
    } else {
        encpriv->mErrorType = Encryption::NoError;
    }

    foreach(const auto &recipient, part->mDecryptRecipients) {
        std::vector<GpgME::Key> found_keys;
        const auto &keyid = recipient.keyID();
        GpgME::KeyListResult res = job->exec(QStringList(QLatin1String(keyid)), false, found_keys);
        if (res.error()) {
            qWarning() << "Error while searching key for Fingerprint: " << keyid;
            continue;
        }
        if (found_keys.size() > 1) {
            // Should not Happen
            qWarning() << "Oops: Found more then one Key for Fingerprint: " << keyid;
        }
        if (found_keys.size() != 1) {
            // Should not Happen at this point
            qWarning() << "Oops: Found no Key for Fingerprint: " << keyid;
            auto keypriv = new KeyPrivate;
            keypriv->mKeyID = keyid;
            encpriv->mRecipients.push_back(Key::Ptr(new Key(keypriv)));
        } else {
            auto key = found_keys[0];
            auto keypriv = new KeyPrivate;
            keypriv->mKey = key;
            encpriv->mRecipients.push_back(Key::Ptr(new Key(keypriv)));
        }
    }
    return Encryption::Ptr(new Encryption(encpriv));
}

void PartPrivate::appendEncryption(const MimeTreeParser::EncryptedMessagePart::Ptr& part)
{
    mEncryptions.append(createEncryption(part));
}

void PartPrivate::setEncryptions(const QVector< Encryption::Ptr >& encs)
{
    mEncryptions = encs;
}

QVector<Signature::Ptr> PartPrivate::createSignature(const MimeTreeParser::SignedMessagePart::Ptr& part)
{
    QVector<Signature::Ptr> sigs;
    QGpgME::KeyListJob *job = part->mCryptoProto->keyListJob(false);    // local, no sigs
    if (!job) {
        qWarning() << "The Crypto backend does not support listing keys. ";
        return sigs;
    }

    foreach(const auto &sig, part->mSignatures) {
        auto sigpriv = new SignaturePrivate();
        sigpriv->mSignature = sig;
        auto signature = std::make_shared<Signature>(sigpriv);
        sigs.append(signature);

        std::vector<GpgME::Key> found_keys;
        const auto &keyid = sig.fingerprint();
        GpgME::KeyListResult res = job->exec(QStringList(QLatin1String(keyid)), false, found_keys);
        if (res.error()) {
            qWarning() << "Error while searching key for Fingerprint: " << keyid;
            continue;
        }
        if (found_keys.size() > 1) {
            // Should not Happen
            qWarning() << "Oops: Found more then one Key for Fingerprint: " << keyid;
            continue;
        }
        if (found_keys.size() != 1) {
            // Should not Happen at this point
            qWarning() << "Oops: Found no Key for Fingerprint: " << keyid;
            continue;
        } else {
            auto key = found_keys[0];
            auto keypriv = new KeyPrivate;
            keypriv->mKey = key;
            sigpriv->mKey = Key::Ptr(new Key(keypriv));
        }
    }
    return sigs;
}

void PartPrivate::appendSignature(const MimeTreeParser::SignedMessagePart::Ptr& part)
{
    mSignatures.append(createSignature(part));
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
    mEncryptions.append(PartPrivate::createEncryption(enc));
}

void ContentPrivate::appendSignature(const MimeTreeParser::SignedMessagePart::Ptr& sig)
{
    mSignatures.append(PartPrivate::createSignature(sig));
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
    return QString::fromUtf8(content());
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
    void fillFrom(const MimeTreeParser::TextMessagePart::Ptr &part);
    void fillFrom(const MimeTreeParser::HtmlMessagePart::Ptr &part);
    void fillFrom(const MimeTreeParser::AttachmentMessagePart::Ptr &part);
    void createEncryptionFailBlock(const MimeTreeParser::EncryptedMessagePart::Ptr &part);
    SinglePart *q;

    QVector<Content::Ptr> mContent;
    QByteArray mType;
};

void SinglePartPrivate::fillFrom(const MimeTreeParser::TextMessagePart::Ptr &part)
{
    mType = "plaintext";
    mContent.clear();
    foreach (const auto &mp, part->subParts()) {
        auto d_ptr = new ContentPrivate;
        d_ptr->mContent = mp->text().toUtf8();
        d_ptr->mParent = q;
        const auto enc = mp.dynamicCast<MimeTreeParser::EncryptedMessagePart>();
        auto sig = mp.dynamicCast<MimeTreeParser::SignedMessagePart>();
        if (enc) {
           d_ptr->appendEncryption(enc);
           if (!enc->isDecryptable()) {
               d_ptr->mContent = QByteArray();
           }
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
        d_ptr->mCodec = q->mailMime()->charset();
    }
}

void SinglePartPrivate::fillFrom(const MimeTreeParser::HtmlMessagePart::Ptr &part)
{
    mType = "html";
    mContent.clear();
    mContent.append(std::make_shared<HtmlContent>(part->text().toUtf8(), q));
    q->reachParentD()->createMailMime(part);
}

void SinglePartPrivate::fillFrom(const MimeTreeParser::AttachmentMessagePart::Ptr &part)
{
    QMimeDatabase mimeDb;
    q->reachParentD()->createMailMime(part.staticCast<MimeTreeParser::TextMessagePart>());
    const auto mimetype = q->mailMime()->mimetype();
    const auto content = q->mailMime()->decodedContent();
    mContent.clear();
    if (mimetype == mimeDb.mimeTypeForName("text/plain")) {
        mType = "plaintext";
        mContent.append(std::make_shared<PlainTextContent>(content, q));
    } else if (mimetype == mimeDb.mimeTypeForName("text/html")) {
        mType = "html";
        mContent.append(std::make_shared<HtmlContent>(content, q));
    } else {
        mType = mimetype.name().toUtf8();
        mContent.append(std::make_shared<Content>(content, q));
    }
}

void SinglePartPrivate::createEncryptionFailBlock(const MimeTreeParser::EncryptedMessagePart::Ptr &part)
{
    mType = "plaintext";
    mContent.clear();
    mContent.append(std::make_shared<PlainTextContent>(QByteArray(), q));
    q->reachParentD()->createMailMime(part);
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
                    if (!enc->isDecryptable()) {
                        auto part = std::make_shared<SinglePart>();
                        part->d->createEncryptionFailBlock(enc);
                        part->reachParentD()->setEncryptions(subTree->d->encryptions());
                        tree->d->appendSubPart(part);
                        return;
                    }
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

Part::Ptr Parser::getPart(const QUrl &url)
{
    if (url.scheme() == QStringLiteral("cid") && !url.path().isEmpty()) {
        const auto cid = url.path();
        return find(d->mTree, [&cid](const Part::Ptr &p){
             const auto mime = p->mailMime();
             return mime->cid() == cid;
        });
    }
    return Part::Ptr();
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

                                    const auto ctname = mime->mimetype().name().trimmed().toLower();
                                    bool mightContent = (content->type() != "Content");     //Content we understand

                                    const auto cd = mime->disposition();
                                    if (cd && cd == MailMime::Inline) {
                                        return mightContent;
                                    }

                                    if (cd && cd == MailMime::Attachment) {
                                        return false;
                                    }

                                    if ((ctname.startsWith("text/") || ctname.isEmpty()) &&
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
                                        QMimeDatabase mimeDb;
                                        auto _mime = content->parent()->mailMime();
                                        const auto parent = _mime->parent();
                                        if (parent) {
                                            const auto mimetype = parent->mimetype();
                                            if (mimetype == mimeDb.mimeTypeForName("multipart/related")) {
                                                return false;
                                            }
                                        }
                                        while (_mime) {
                                            if (_mime && (_mime->isTopLevelPart() || _mime->isFirstTextPart())) {
                                                return false;
                                            }
                                            if (_mime->isFirstPart()) {
                                                _mime = _mime->parent();
                                            } else {
                                                break;
                                            }
                                        }
                                    }

                                    const auto ctname = mime->mimetype().name().trimmed().toLower();
                                    bool mightContent = (content->type() != "Content");         //Content we understand

                                    const auto cd = mime->disposition();
                                    if (cd && cd == MailMime::Inline) {
                                        // explict "inline" disposition:
                                        return !mightContent;
                                    }
                                    if (cd && cd == MailMime::Attachment) {
                                        // explicit "attachment" disposition:
                                        return true;
                                    }

                                    const auto ct = mime->mimetype();
                                    if ((ctname.startsWith("text/") || ctname.isEmpty()) &&
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

Part::Ptr Parser::find(const Part::Ptr &start, std::function<bool(const Part::Ptr &)> select) const
{
    foreach (const auto &part, start->subParts()) {
        if (select(part)) {
            return part;
        }
        const auto ret = find(part, select);
        if (ret) {
            return ret;
        }
    }
    return Part::Ptr();
}
