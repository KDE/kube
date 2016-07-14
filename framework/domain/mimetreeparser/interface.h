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

#include <QUrl>
#include <QMimeType>

class Part;
typedef std::shared_ptr<Part> Part::Ptr;
class EncryptionPart;
typedef std::shared_ptr<Part> EncryptionPart::Ptr;
class SignaturePart;
typedef std::shared_ptr<Part> SignaturePart::Ptr;

class MimePart;
typedef std::shared_ptr<Part> MimePart::Ptr;
class MimePartPrivate;

class ContentPart;
typedef std::shared_ptr<Part> ContentPart::Ptr;
class ContentPartPrivate;

class EncryptionErrorPart;
typedef std::shared_ptr<Part> EncryptionErrorPart::Ptr;
class EncryptionErrorPartPrivate;

class AttachmentPart;
typedef std::shared_ptr<Part> AttachmentPart::Ptr;
class AttachmentPartPrivate;

class EncapsulatedPart;
typedef std::shared_ptr<Part> EncapsulatedPart::Ptr;
class EncapsulatedPart;

class CertPart;
typedef std::shared_ptr<Part> CertPart::Ptr;

class Key;
class Signature;
class Encryption;

class Parser;
typedef std::shared_ptr<Parser> Parser::Ptr;
class ParserPrivate;

class Parser
{
public:
    Parser(const QByteArray &mimeMessage);

    Part::Ptr getPart(QUrl url);

    QVector<AttachmentPart::Ptr> collect<AttachmentPart>() const;
    QVector<ContentPart:Ptr> collect<ContentPart>() const;
    QVector<T::Ptr> collect<T>(Part start, std::function<bool(const Part &)> select, std::function<bool(const T::Ptr &)> filter) const;

private:
    std::unique_ptr<ParserPrivate> d;
};


class Part
{
public:
    virtual QByteArray type() const = 0;

    bool hasSubParts() const;
    QList<Part::Ptr> subParts() const;
    Part parent() const;

    virtual QVector<Signature> signatures() const;
    virtual QVector<Encryption> encryptions() const;
};

//A structure element, that we need to reflect, that there is a Encryption starts
// only add a new Encrption block to encryptions block
class EncryptionPart : public Part
{
public:
    QVector<Encryption> encryptions() const Q_DECL_OVERRIDE;
    QByteArray type() const Q_DECL_OVERRIDE;
};

// A structure element, that we need to reflect, that there is a Signature starts
// only add a new Signature block to signature block
// With this we can a new Singature type like pep aka
/*
 * add a bodypartformatter, that returns a PEPSignaturePart with all signed subparts that are signed with pep.
 * subclass Signature aka PEPSignature to reflect different way of properties of PEPSignatures.
 */
class SignaturePart : public Part
{
public:
    QVector<Signature> signatures() const Q_DECL_OVERRIDE;
    QByteArray type() const Q_DECL_OVERRIDE;
};



class TextPart : public Part
{
public:
    QByteArray content() const;

    //Use default charset
    QString encodedContent() const;

    // overwrite default charset with given charset
    QString encodedContent(QByteArray charset) const;
}

/* 
 * A MessagePart that is based on a KMime::Content
 */
class MimePart : public TextPart
{
public:
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
class MainContentPart : public MimePart
{
public:
    enum Types {
        PlainText,
        Html
    };
    Q_DECLARE_FLAGS(Types, Type)

    QVector<TextPart> content(Content::Type ct) const;

    Content::Types availableContent() const;

    QByteArray type() const Q_DECL_OVERRIDE;

private:
    std::unique_ptr<ContentPartPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ContentPart::Type)

class AttachmentPart : public MimePart
{
public:
    QByteArray type() const Q_DECL_OVERRIDE;

private:
    std::unique_ptr<AttachmentPartPrivate> d;
};

/*
 * Faild to decrypt part
 * thigs liks this can happen:
 * decryption in progress
 * have not tried at all to decrypt
 * wrong passphrase
 * no private key
 * cryptobackend is not configured correctly (no gpg available) 
 *  -> S/Mime and PGP have different meaning in their errors
 * 
 * Open Questions:
 * - How to make the string translateable for multiple clients, so that multiple clients can show same error messages,
 * that helps users to understand what is going on ?
 * - Does openpgp have translations already?
 */
class EncryptionErrorPart : public Part
{
public:
    Error errorId() const;

    CryptoBackend cryptoBackend();

    QByteArray type() const Q_DECL_OVERRIDE;

private:
    std::unique_ptr<EncryptionErrorPartPrivate> d;
};

/*
 * we want to request complete headers like:
 * from/to...
 */

class EncapsulatedPart :: public AttachmentPart
{
public:
    QByteArray type() const Q_DECL_OVERRIDE;

    QByteArray header<Type>();
private:
    std::unique_ptr<EncryptionErrorPartPrivate> d;    
};

/*
 * importing a cert GpgMe::ImportResult
 * checking a cert (if it is a valid cert)
 */

class CertPart :: public AttachmentPart
{
public:
    QByteArray type() const Q_DECL_OVERRIDE;

    bool checkCert() const;
    Status importCert() const;

private:
    std::unique_ptr<CertPartPrivate> d;    
};

/*
the ggme error class

// class GPGMEPP_EXPORT ErrorImportResult
{
public:
    Error() : mErr(0), mMessage() {}
    explicit Error(unsigned int e) : mErr(e), mMessage() {}

    const char *source() const;
    const char *asString() const;

    int code() const;
    int sourceID() const;

    bool isCanceled() const;

    unsigned int encodedError() const
    {
        return mErr;
    }
    int toErrno() const;

    static bool hasSystemError();
    static Error fromSystemError(unsigned int src = GPGMEPP_ERR_SOURCE_DEFAULT);
    static void setSystemError(gpg_err_code_t err);
    static void setErrno(int err);
    static Error fromErrno(int err, unsigned int src = GPGMEPP_ERR_SOURCE_DEFAULT);
    static Error fromCode(unsigned int err, unsigned int src = GPGMEPP_ERR_SOURCE_DEFAULT);

    GPGMEPP_MAKE_SAFE_BOOL_OPERATOR(mErr  &&!isCanceled())
private:
    unsigned int mErr;
    mutable std::string mMessage;
};
*/

/*
 * a used smime/PGP key
 * in the end we also need things like:
   bool isRevokation() const;
   bool isInvalid() const;
   bool isExpired() const;

   -> so we end up wrapping GpgME::Key
 */
class Key
{
    QString keyid() const;
    QString name() const;
    QString email() const;
    QString comment() const;
    QVector<QString> emails() const;
    KeyTrust keyTrust() const;
    CryptoBackend cryptoBackend() const;

    std::vector<Key> subkeys();
    Key parentkey() const;
};

class Signature
{
    Key key() const;
    QDateTime creationDateTime() const;
    QDateTime expirationTime() const;
    bool neverExpires() const;

    bool inProgress();      //if the verfication is inProgress

    enum Validity {
        Unknown, Undefined, Never, Marginal, Full, Ultimate
    };
    Validity validity() const;

    // to determine if we need this in our usecase (email)
    // GpgME::VerificationResult
    enum Summary {
        None       = 0x000,
        Valid      = 0x001,
        Green      = 0x002,
        Red        = 0x004,
        KeyRevoked = 0x008,
        KeyExpired = 0x010,
        SigExpired = 0x020,
        KeyMissing = 0x040,
        CrlMissing = 0x080,
        CrlTooOld  = 0x100,
        BadPolicy  = 0x200,
        SysError   = 0x400
    };
    Summary summary() const;

    const char *policyURL() const;
    GpgME::Notation notation(unsigned int index) const;
    std::vector<GpgME::Notation> notations() const;

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