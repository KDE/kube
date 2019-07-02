/*
   Copyright (c) 2015 Sandro Knauß <sknauss@kde.org>

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

#ifndef __MIMETREEPARSER_MESSAGEPART_H__
#define __MIMETREEPARSER_MESSAGEPART_H__

#include "util.h"
#include "enums.h"
#include "partmetadata.h"
#include <crypto.h>

#include <KMime/Message>

#include <QString>
#include <QSharedPointer>

class QTextCodec;
class PartPrivate;

namespace KMime
{
class Content;
}

namespace MimeTreeParser
{
class ObjectTreeParser;
class HTMLBlock;
typedef QSharedPointer<HTMLBlock> HTMLBlockPtr;
class CryptoBodyPartMemento;
class MultiPartAlternativeBodyPartFormatter;

class SignedMessagePart;
class EncryptedMessagePart;

using Crypto::CryptoProtocol;
using Crypto::CryptoProtocol::CMS;
using Crypto::CryptoProtocol::OpenPGP;
using Crypto::CryptoProtocol::UnknownProtocol;

class MessagePart : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool attachment READ isAttachment)
    Q_PROPERTY(bool root READ isRoot)
    Q_PROPERTY(bool isHtml READ isHtml)
    Q_PROPERTY(QString plaintextContent READ plaintextContent)
    Q_PROPERTY(QString htmlContent READ htmlContent)
public:
    enum Disposition {
        Inline,
        Attachment,
        Invalid
    };
    typedef QSharedPointer<MessagePart> Ptr;
    MessagePart(ObjectTreeParser *otp, const QString &text, KMime::Content *node = nullptr);

    virtual ~MessagePart();

    virtual QString text() const;
    void setText(const QString &text);
    virtual bool isAttachment() const;

    void setIsRoot(bool root);
    bool isRoot() const;

    void setParentPart(MessagePart *parentPart);
    MessagePart *parentPart() const;

    virtual QString plaintextContent() const;
    virtual QString htmlContent() const;

    virtual bool isHtml() const;

    QByteArray mimeType() const;
    QByteArray charset() const;
    QString filename() const;
    Disposition disposition() const;
    bool isText() const;

    enum Error {
        NoError = 0,
        PassphraseError,
        NoKeyError,
        UnknownError
    };

    Error error() const;
    QString errorString() const;

    PartMetaData *partMetaData();

    void appendSubPart(const MessagePart::Ptr &messagePart);
    const QVector<MessagePart::Ptr> &subParts() const;
    bool hasSubParts() const;

    KMime::Content *node() const;

    virtual KMMsgSignatureState signatureState() const;
    virtual KMMsgEncryptionState encryptionState() const;

    QVector<SignedMessagePart*> signatures() const;
    QVector<EncryptedMessagePart*> encryptions() const;

    void bindLifetime(KMime::Content *);

protected:
    void parseInternal(KMime::Content *node, bool onlyOneMimePart = false);
    void parseInternal(const QByteArray &data);
    QString renderInternalText() const;

    QString mText;
    ObjectTreeParser *mOtp;
    PartMetaData mMetaData;
    MessagePart *mParentPart;
    KMime::Content *mNode;
    QVector<KMime::Content*> mNodesToDelete;
    Error mError;

private:
    QVector<MessagePart::Ptr> mBlocks;
    bool mRoot;
};

class MimeMessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<MimeMessagePart> Ptr;
    MimeMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node, bool onlyOneMimePart = false);
    virtual ~MimeMessagePart();

    QString text() const Q_DECL_OVERRIDE;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;
private:
    friend class AlternativeMessagePart;
    friend class ::PartPrivate;
};

class MessagePartList : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<MessagePartList> Ptr;
    MessagePartList(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node = nullptr);
    virtual ~MessagePartList();

    QString text() const Q_DECL_OVERRIDE;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;
};

enum IconType {
    NoIcon = 0,
    IconExternal,
    IconInline
};

class TextMessagePart : public MessagePartList
{
    Q_OBJECT
public:
    typedef QSharedPointer<TextMessagePart> Ptr;
    TextMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node);
    virtual ~TextMessagePart();

    KMMsgSignatureState signatureState() const Q_DECL_OVERRIDE;
    KMMsgEncryptionState encryptionState() const Q_DECL_OVERRIDE;

private:
    void parseContent();

    KMMsgSignatureState mSignatureState;
    KMMsgEncryptionState mEncryptionState;

    friend class DefaultRendererPrivate;
    friend class ObjectTreeParser;
    friend class ::PartPrivate;
};

class AttachmentMessagePart : public TextMessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<AttachmentMessagePart> Ptr;
    AttachmentMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node);
    virtual ~AttachmentMessagePart();
    virtual bool isAttachment() const Q_DECL_OVERRIDE { return true; }

};

class HtmlMessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<HtmlMessagePart> Ptr;
    HtmlMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node);
    virtual ~HtmlMessagePart();

    QString text() const Q_DECL_OVERRIDE;

    bool isHtml() const Q_DECL_OVERRIDE;

private:
    QString mBodyHTML;
    QByteArray mCharset;

    friend class DefaultRendererPrivate;
    friend class ::PartPrivate;
};

class AlternativeMessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<AlternativeMessagePart> Ptr;
    AlternativeMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node);
    virtual ~AlternativeMessagePart();

    QString text() const Q_DECL_OVERRIDE;

    bool isHtml() const Q_DECL_OVERRIDE;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;
    QString icalContent() const;

    QList<Util::HtmlMode> availableModes();
private:
    QMap<Util::HtmlMode, MessagePart::Ptr> mChildParts;

    friend class DefaultRendererPrivate;
    friend class ObjectTreeParser;
    friend class MultiPartAlternativeBodyPartFormatter;
    friend class ::PartPrivate;
};

class CertMessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<CertMessagePart> Ptr;
    CertMessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node, const CryptoProtocol cryptoProto);
    virtual ~CertMessagePart();

    QString text() const Q_DECL_OVERRIDE;
    void import();

private:
    const CryptoProtocol mProtocol;
    friend class DefaultRendererPrivate;
};

class EncapsulatedRfc822MessagePart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<EncapsulatedRfc822MessagePart> Ptr;
    EncapsulatedRfc822MessagePart(MimeTreeParser::ObjectTreeParser *otp, KMime::Content *node, const KMime::Message::Ptr &message);
    virtual ~EncapsulatedRfc822MessagePart();

    QString text() const Q_DECL_OVERRIDE;
    QString from() const;
    QDateTime date() const;
private:
    const KMime::Message::Ptr mMessage;

    friend class DefaultRendererPrivate;
};

class EncryptedMessagePart : public MessagePart
{
    Q_OBJECT
    Q_PROPERTY(bool isEncrypted READ isEncrypted)
public:
    typedef QSharedPointer<EncryptedMessagePart> Ptr;
    EncryptedMessagePart(ObjectTreeParser *otp,
                         const QString &text,
                         const CryptoProtocol protocol,
                         const QString &fromAddress,
                         KMime::Content *node, KMime::Content *encryptedNode = nullptr);

    virtual ~EncryptedMessagePart();

    QString text() const Q_DECL_OVERRIDE;

    void setIsEncrypted(bool encrypted);
    bool isEncrypted() const;

    bool isDecryptable() const;

    void startDecryption(const QByteArray &text, const QTextCodec *aCodec);
    void startDecryption(KMime::Content *data = nullptr);

    QByteArray mDecryptedData;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;

private:
    /** Handles the dectyptioon of a given content
     * returns true if the decryption was successful
     * if used in async mode, check if mMetaData.inProgress is true, it inicates a running decryption process.
     */
    bool okDecryptMIME(KMime::Content &data);

protected:
    const CryptoProtocol mProtocol;
    QString mFromAddress;
    QByteArray mVerifiedText;
    KMime::Content *mEncryptedNode;

    friend class DefaultRendererPrivate;
    friend class ::PartPrivate;
};

class SignedMessagePart : public MessagePart
{
    Q_OBJECT
    Q_PROPERTY(bool isSigned READ isSigned)
public:
    typedef QSharedPointer<SignedMessagePart> Ptr;
    SignedMessagePart(ObjectTreeParser *otp,
                      const QString &text,
                      const CryptoProtocol protocol,
                      const QString &fromAddress,
                      KMime::Content *node, KMime::Content *signedData);

    virtual ~SignedMessagePart();

    void setIsSigned(bool isSigned);
    bool isSigned() const;

    void startVerificationDetached(const QByteArray &text, KMime::Content *textNode, const QByteArray &signature);
    void startVerification();

    QByteArray mDecryptedData;

    QString plaintextContent() const Q_DECL_OVERRIDE;
    QString htmlContent() const Q_DECL_OVERRIDE;

private:
    void sigStatusToMetaData(const Crypto::Signature &signature);
    void setVerificationResult(const Crypto::VerificationResult &result, bool parseText, const QByteArray &plainText);

protected:
    CryptoProtocol mProtocol;
    QString mFromAddress;
    KMime::Content *mSignedData;

    friend EncryptedMessagePart;
    friend class DefaultRendererPrivate;
    friend class ::PartPrivate;
};

class HeadersPart : public MessagePart
{
    Q_OBJECT
public:
    typedef QSharedPointer<HeadersPart> Ptr;
    HeadersPart(ObjectTreeParser *otp, KMime::Content *node);
    virtual ~HeadersPart();
};

}

#endif //__MIMETREEPARSER_MESSAGEPART_H__
