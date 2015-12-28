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

#ifndef _MESSAGEVIEWER_MESSAGEPART_H_
#define _MESSAGEVIEWER_MESSAGEPART_H_

#include "partmetadata.h"
#include "nodehelper.h"
#include <KMime/Message>

#include <Libkleo/CryptoBackend>
#include <gpgme++/verificationresult.h>
#include <importresult.h>

#include <QString>
#include <QSharedPointer>

class QTextCodec;

namespace GpgME
{
    class ImportResult;
}

namespace KMime
{
class Content;
}

namespace MessageViewer
{
class ObjectTreeParser;
class ObjectTreeSourceIf;
class HtmlWriter;
class NodeHelper;

class HTMLBlock
{
public:
    typedef QSharedPointer<HTMLBlock> Ptr;

    HTMLBlock()
        : entered(false)
    { }

    virtual ~HTMLBlock() { }

protected:
    bool entered;
};

class CryptoBlock: public HTMLBlock
{
public:
    CryptoBlock(ObjectTreeParser *otp,
                PartMetaData *block,
                const Kleo::CryptoBackend::Protocol *cryptoProto,
                const QString &fromAddress,
                KMime::Content *node);
    virtual ~CryptoBlock();

private:
    void internalEnter();
    void internalExit();

    ObjectTreeParser *mOtp;
    PartMetaData *mMetaData;
    const Kleo::CryptoBackend::Protocol *mCryptoProto;
    QString mFromAddress;
    KMime::Content *mNode;
};

// The attachment mark is a div that is placed around the attchment. It is used for drawing
// a yellow border around the attachment when scrolling to it. When scrolling to it, the border
// color of the div is changed, see KMReaderWin::scrollToAttachment().
class AttachmentMarkBlock : public HTMLBlock
{
public:
    AttachmentMarkBlock(MessageViewer::HtmlWriter *writer, KMime::Content *node);
    virtual ~AttachmentMarkBlock();

private:
    void internalEnter();
    void internalExit();

    KMime::Content *mNode;
    HtmlWriter *mWriter;
};

class TextBlock : public HTMLBlock
{
public:
    TextBlock(MessageViewer::HtmlWriter *writer, MessageViewer::NodeHelper *nodeHelper, KMime::Content *node, bool link);
    virtual ~TextBlock();
private:
    void internalEnter();
    void internalExit();
private:
    HtmlWriter *mWriter;
    NodeHelper *mNodeHelper;
    KMime::Content *mNode;
    QString mFileName;
    bool mLink;
};

class HTMLWarnBlock : public HTMLBlock
{
public:
    HTMLWarnBlock(MessageViewer::HtmlWriter* writer, const QString &msg);
    virtual ~HTMLWarnBlock();
private:
    void internalEnter();
    void internalExit();
private:
    HtmlWriter* mWriter;
    const QString &mMsg;
};

class MessagePart
{
public:
    typedef QSharedPointer<MessagePart> Ptr;
    MessagePart(ObjectTreeParser *otp,
                const QString &text);

    virtual ~MessagePart();

    virtual QString text() const;
    void setText(const QString &text);
    virtual void html(bool decorate);

    PartMetaData *partMetaData();

protected:
    void parseInternal(KMime::Content *node, bool onlyOneMimePart);
    void renderInternalHtml() const;
    void copyContentFrom() const;
    QString renderInternalText() const;
    QString mText;
    ObjectTreeParser *mOtp;
    ObjectTreeParser *mSubOtp;
    PartMetaData mMetaData;
};

class MimeMessagePart : public MessagePart
{
public:
    typedef QSharedPointer<MimeMessagePart> Ptr;
    MimeMessagePart(MessageViewer::ObjectTreeParser *otp, KMime::Content *node, bool onlyOneMimePart);
    virtual ~MimeMessagePart();

    QString text() const Q_DECL_OVERRIDE;
    void html(bool decorate) Q_DECL_OVERRIDE;

private:
    KMime::Content *mNode;
    bool mOnlyOneMimePart;

    friend class AlternativeMessagePart;
};

class TextMessagePart : public MessagePart
{
public:
    typedef QSharedPointer<TextMessagePart> Ptr;
    TextMessagePart(MessageViewer::ObjectTreeParser *otp, KMime::Content *node, bool drawFrame, bool showLink, bool decryptMessage);
    virtual ~TextMessagePart();

    QString text() const Q_DECL_OVERRIDE;
    void html(bool decorate) Q_DECL_OVERRIDE;

    KMMsgSignatureState signatureState() const;
    KMMsgEncryptionState encryptionState() const;

    bool decryptMessage() const;
private:
    void parseContent();
    KMime::Content *mNode;
    KMMsgSignatureState mSignatureState;
    KMMsgEncryptionState mEncryptionState;
    QVector<MessagePart::Ptr> mBlocks;
    bool mDrawFrame;
    bool mShowLink;
    bool mDecryptMessage;
};

class HtmlMessagePart : public MessagePart
{
public:
    typedef QSharedPointer<HtmlMessagePart> Ptr;
    HtmlMessagePart(MessageViewer::ObjectTreeParser* otp, KMime::Content* node, MessageViewer::ObjectTreeSourceIf* source);
    virtual ~HtmlMessagePart();

    QString text() const Q_DECL_OVERRIDE;
    void html(bool decorate) Q_DECL_OVERRIDE;

    /* only a function that should be removed if the refactiring is over */
    void fix();

private:
    KMime::Content* mNode;
    ObjectTreeSourceIf *mSource;
    QString mBodyHTML;
    QByteArray mCharset;
};

class AlternativeMessagePart : public MessagePart
{
public:
    typedef QSharedPointer<AlternativeMessagePart> Ptr;
    AlternativeMessagePart (MessageViewer::ObjectTreeParser* otp, KMime::Content* textNode, KMime::Content* htmlNode);
    virtual ~AlternativeMessagePart();

    QString text() const Q_DECL_OVERRIDE;
    void html(bool decorate) Q_DECL_OVERRIDE;

    void setViewHtml(bool html);
    bool viewHtml();
private:
    KMime::Content* mTextNode;
    KMime::Content* mHTMLNode;

    MimeMessagePart::Ptr mTextPart;
    MimeMessagePart::Ptr mHTMLPart;
    bool mViewHtml;
};

class CertMessagePart : public MessagePart
{
public:
    typedef QSharedPointer<CertMessagePart> Ptr;
    CertMessagePart(MessageViewer::ObjectTreeParser* otp, KMime::Content* node, const Kleo::CryptoBackend::Protocol *cryptoProto, bool autoImport);
    virtual ~CertMessagePart();

    QString text() const Q_DECL_OVERRIDE;
    void html(bool decorate) Q_DECL_OVERRIDE;

private:
    KMime::Content* mNode;
    bool mAutoImport;
    GpgME::ImportResult mImportResult;
    const Kleo::CryptoBackend::Protocol *mCryptoProto;
};

class EncapsulatedRfc822MessagePart : public MessagePart
{
public:
    typedef QSharedPointer<EncapsulatedRfc822MessagePart> Ptr;
    EncapsulatedRfc822MessagePart(MessageViewer::ObjectTreeParser *otp, KMime::Content *node, const KMime::Message::Ptr &message);
    virtual ~EncapsulatedRfc822MessagePart();

    QString text() const Q_DECL_OVERRIDE;
    void html(bool decorate) Q_DECL_OVERRIDE;

private:
    const KMime::Message::Ptr mMessage;
    KMime::Content *mNode;
};

class CryptoMessagePart : public MessagePart
{
public:
    typedef QSharedPointer<CryptoMessagePart> Ptr;
    CryptoMessagePart(ObjectTreeParser *otp,
                      const QString &text,
                      const Kleo::CryptoBackend::Protocol *cryptoProto,
                      const QString &fromAddress,
                      KMime::Content *node);

    virtual ~CryptoMessagePart();

    void startDecryption(const QByteArray &text, const QTextCodec *aCodec);
    void startDecryption(KMime::Content *data = 0);
    void startVerification(const QByteArray &text, const QTextCodec *aCodec);
    void startVerificationDetached(const QByteArray &text, KMime::Content *textNode, const QByteArray &signature);
    void html(bool decorate) Q_DECL_OVERRIDE;

    bool mPassphraseError;
    QByteArray mDecryptedData;
    std::vector<GpgME::Signature> mSignatures;

private:
    /** Writes out the block that we use when the node is encrypted,
      but we're deferring decryption for later. */
    void writeDeferredDecryptionBlock() const;

protected:
    const Kleo::CryptoBackend::Protocol *mCryptoProto;
    QString mFromAddress;
    KMime::Content *mNode;
    bool mDecryptMessage;
    QByteArray mVerifiedText;
};

}
#endif //_MESSAGEVIEWER_MESSAGEPART_H_
