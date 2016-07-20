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

#pragma once

#include <functional>
#include <memory>

#include <QDateTime>
#include <QUrl>
#include <QMimeType>

class Part;
class PartPrivate;

class MimePart;
class MimePartPrivate;

class ContentPart;
class ContentPartPrivate;

class EncryptionPart;
class EncryptionPartPrivate;

class AttachmentPart;
class AttachmentPartPrivate;

class EncapsulatedPart;
class EncapsulatedPartPrivate;

class CertPart;
class CertPartPrivate;

class Content;
class ContentPrivate;

class Key;
class Signature;
class Encryption;

class Parser;
class ParserPrivate;

class Part
{
public:
    typedef std::shared_ptr<Part> Ptr;
    Part();
    virtual QByteArray type() const;

    bool hasSubParts() const;
    QVector<Part::Ptr> subParts() const;
    Part *parent() const;

    virtual QVector<Signature> signatures() const;
    virtual QVector<Encryption> encryptions() const;
private:
    std::unique_ptr<PartPrivate> d;
    friend class ParserPrivate;
    friend class PartPrivate;
};

class Content
{
public:
    typedef std::shared_ptr<Content> Ptr;
    Content(const QByteArray &content, ContentPart *parent);
    virtual ~Content();

    QByteArray content() const;

    QByteArray charset() const;

    //Use default charset
    QString encodedContent() const;

    // overwrite default charset with given charset
    QString encodedContent(QByteArray charset) const;

    virtual QVector<Signature> signatures() const;
    virtual QVector<Encryption> encryptions() const;
private:
    std::unique_ptr<ContentPrivate> d;
};

/* 
 * A MessagePart that is based on a KMime::Content
 */
class MimePart : public Part
{
public:
    typedef std::shared_ptr<MimePart> Ptr;
    /**
     *  Various possible values for the "Content-Disposition" header.
     */
    enum Disposition {
        Invalid,           ///< Default, invalid value
        Inline,            ///< inline
        Attachment        ///< attachment
    };

    // interessting header parts of a KMime::Content
    QMimeType mimetype() const;
    Disposition disposition() const;
    QUrl label() const;
    QByteArray cid() const;
    QByteArray charset() const;

    // we wanna overrwrite the charset of the content, because some clients set the charset wrong
    void setCharset(QByteArray charset);

    // Unique identifier to ecactly this KMime::Content
    QByteArray link() const;

    QByteArray content() const;

    //Use default charset
    QString encodedContent() const;

    // overwrite default charset with given charset
    QString encodedContent(QByteArray charset) const;

    QByteArray type() const Q_DECL_OVERRIDE;
private:
    std::unique_ptr<MimePartPrivate> d;
};

/*
 * The main ContentPart
 * is MimePart a good parent class?
 * do we wanna need parts of the header of the connected KMime::Contents
 * usecases:
 *  - 
 * for htmlonly it is representating only one MimePart (ok)
 * for plaintext only also only one MimePart (ok)
 * for alternative, we are represating three messageparts
 *   - "headers" do we return?, we can use setType to make it possible to select and than return these headers
 */
class ContentPart : public Part
{
public:
    typedef std::shared_ptr<ContentPart> Ptr;
    enum Type {
        PlainText = 0x0001,
        Html      = 0x0002
    };
    Q_DECLARE_FLAGS(Types, Type)

    ContentPart();
    virtual ~ContentPart();

    QVector<Content::Ptr> content(Type ct) const;

    Types availableContents() const;

    QByteArray type() const Q_DECL_OVERRIDE;

private:
    std::unique_ptr<ContentPartPrivate> d;

    friend class ParserPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ContentPart::Types);

class AttachmentPart : public MimePart
{
public:
    typedef std::shared_ptr<AttachmentPart> Ptr;
    QByteArray type() const Q_DECL_OVERRIDE;

private:
    std::unique_ptr<AttachmentPartPrivate> d;

    friend class ParserPrivate;
};

/*
 * Open Questions:
 * - How to make the string translateable for multiple clients, so that multiple clients can show same error messages,
 * that helps users to understand what is going on ?
 * - Does openpgp have translations already?
 */
class EncryptionError
{
public:
    int errorId() const;
    QString errorString() const;
};

class EncryptionPart : public MimePart
{
public:
    typedef std::shared_ptr<EncryptionPart> Ptr;
    QByteArray type() const Q_DECL_OVERRIDE;

    EncryptionError error() const;

private:
    std::unique_ptr<EncryptionPartPrivate> d;    
};


/*
 * we want to request complete headers like:
 * from/to...
 */

class EncapsulatedPart : public AttachmentPart
{
public:
    typedef std::shared_ptr<EncapsulatedPart> Ptr;
    QByteArray type() const Q_DECL_OVERRIDE;

    //template <class T> QByteArray header<T>();
private:
    std::unique_ptr<EncapsulatedPartPrivate> d;    
};

/*
 * importing a cert GpgMe::ImportResult
 * checking a cert (if it is a valid cert)
 */

class CertPart : public AttachmentPart
{
public:
    typedef std::shared_ptr<CertPart> Ptr;
    QByteArray type() const Q_DECL_OVERRIDE;

    enum CertType {
        Pgp,
        SMime
    };

    enum CertSubType {
        Public,
        Private
    };

    CertType certType() const;
    CertSubType certSubType() const;
    int keyLength() const;

private:
    std::unique_ptr<CertPartPrivate> d;    
};

class Key
{
    QString keyid() const;
    QString name() const;
    QString email() const;
    QString comment() const;
    QVector<QString> emails() const;
    enum KeyTrust {
        Unknown, Undefined, Never, Marginal, Full, Ultimate
    };
    KeyTrust keyTrust() const;

    bool isRevokation() const;
    bool isInvalid() const;
    bool isExpired() const;

    std::vector<Key> subkeys();
    Key parentkey() const;
};

class Signature
{
    Key key() const;
    QDateTime creationDateTime() const;
    QDateTime expirationTime() const;
    bool neverExpires() const;

    //template <> StatusObject<SignatureVerificationResult> verify() const;
};

/*
 * Normally the Keys for encryption are subkeys
 * for clients the parentkeys are "more interessting", because they store the name, email etc.
 * but a client may also wants show to what subkey the mail is really encrypted, an if this subkey isRevoked or something else
 */
class Encryption
{
    std::vector<Key> recipients() const;
};

class Parser
{
public:
    typedef std::shared_ptr<Parser> Ptr;
    Parser(const QByteArray &mimeMessage);
    ~Parser();

    Part::Ptr getPart(QUrl url);

    template <typename T> QVector<typename T::Ptr> collect(const Part::Ptr &start, std::function<bool(const Part::Ptr &)> select, std::function<bool(const typename T::Ptr &)> filter) const;
    QVector<AttachmentPart::Ptr> collectAttachments(Part::Ptr start, std::function<bool(const Part::Ptr &)> select, std::function<bool(const AttachmentPart::Ptr &)> filter) const;
    ContentPart::Ptr collectContentPart(Part::Ptr start, std::function<bool(const Part::Ptr &)> select, std::function<bool(const ContentPart::Ptr &)> filter) const;
    ContentPart::Ptr collectContentPart(const Part::Ptr& start) const;
    ContentPart::Ptr collectContentPart() const;
    //template <> QVector<ContentPart::Ptr> collect<ContentPart>() const;

    //template <> static StatusObject<SignatureVerificationResult> verifySignature(const Signature signature) const;
    //template <> static StatusObject<Part> decrypt(const EncryptedPart part) const;

signals:
    void partsChanged();

private:
    std::unique_ptr<ParserPrivate> d;

    friend class InterfaceTest;
};

