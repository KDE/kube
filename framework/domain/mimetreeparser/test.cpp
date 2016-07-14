Usecases:

# plaintext msg + attachment
* ContentPart => cp1
* AttachmentPart => ap1

(cp1) == collect<ContentPart>(select=NoEncapsulatedMessages)
(ap1) == collect<AttachmentParts>(select=NoEncapsulatedMessages)

(PlainText) == cp1.availableContent()

# html msg + related attachment + normal attachment
* ContentPart => cp1
* AttachmentPart(mimetype="*/related", cid="12345678") => ap1
* AttachmentPart => ap2

(cp1) == collect<ContentPart>(select=NoEncapsulatedMessages)
(ap1, ap2) == collect<AttachmentParts>(select=NoEncapsulatedMessages)
(ap2) == collect<AttachmentParts>(select=NoEncapsulatedMessages, filter=filterelated)

ap1 == getPart("cid:12345678")

(Html) == cp1.availableContent()

# alternative msg + attachment
* ContentPart(html="HTML", plaintext="Text") => cp1
* AttachmentPart => ap1

(cp1) == collect<ContentPart>(select=NoEncapsulatedMessages)
(ap1) == collect<AttachmentParts>(select=NoEncapsulatedMessages)

(Html, PlainText) == cp1.availableContent()
"HTML" == cp1.content(Html)
"text" == cp1.content(Plaintext)

# alternative msg with GPGInline
* ContentPart(html="HTML", plaintext="Text cypted<foo>") => cp1
    * TextPart(text="Text")
    * TextPart(text=foo, encryption=(enc1)

(Html, PlainText) == cp1.availableContent()

TODO: but how to get plaintext/html content?

# encrypted msg (not encrypted/error) with unencrypted attachment
* EncryptionErrorPart => cp1
* AttachmentPart => ap1

(cp1) == collect<ContentPart>(select=NoEncapsulatedMessages)
(ap1) == collect<AttachmentParts>(select=NoEncapsulatedMessages)

#encrypted msg (decrypted with attachment) + unencrypted attachment
* encrytion=(rec1,rec2) => enc1
    * ContentPart(encrytion = (enc1,)) => cp1 
    * AttachmentPart(encryption = (enc1,)) => ap1
* AttachmentPart => ap2

(cp1) == collect<ContentPart>(select=NoEncapsulatedMessages)
(ap1, ap2) == collect<AttachmentParts>(select=NoEncapsulatedMessages)

#INLINE GPG encrypted msg + attachment
* ContentPart => cp1
  * TextPart
  * TextPart(encrytion = (enc1(rec1,rec2),))
  * TextPart(signed = (sig1,))
  * TextPart
* AttachmentPart => ap1

(cp1) == collect<ContentPart>(select=NoEncapsulatedMessages)
(ap1) == collect<AttachmentParts>(select=NoEncapsulatedMessages)

#forwared encrypted msg + attachments
* ContentPart => cp1
* EncapsulatedPart => ep1
    * Encrytion=(rec1,rec2) => enc1
        * Signature => sig1
            * ContentPart(encrytion = (enc1,), signature = (sig1,)) => cp2
                * TextPart(encrytion = (enc1,), signature = (sig1,))
                * TextPart(encrytion = (enc1, enc2(rec3,rec4),), signature = (sig1,))
            * AttachmentPart(encrytion = (enc1,), signature = (sig1,)) => ap1
* AttachmentPart => ap2

(cp1) = collect<ContentPart>(select=NoEncapsulatedMessages)
(ap2) = collect<AttachmentParts>(select=NoEncapsulatedMessages)

(cp2) = collect<ContentPart>(ep1, select=NoEncapsulatedMessages)
(ap1) = collect<AttachmentParts>(ep1, select=NoEncapsulatedMessages)

(cp1, cp2) == collect<ContentPart>()
(ap1, ap2) == collect<AttachmentParts>()


# plaintext msg + attachment + cert
* ContentPart => cp1
* AttachmentPart => ap1
* CertPart => cep1

(cp1) == collect<ContentPart>(select=NoEncapsulatedMessages)
(ap1, cep1) == collect<AttachmentPart>(select=NoEncapsulatedMessages)
(ap1) == collect<AttachmentPart>(select=NoEncapsulatedMessages, filter=filterSubAttachmentParts)

(cep1) == collect<CertPart>(select=NoEncapsulatedMessages)


collect function:

bool noEncapsulatedMessages(Part part)
{
    if (is<EncapsulatedPart>(part)) {
        return false;
    }
    return true;
}

bool filterRelated(T part)
{
    if (part.mimetype == related && !part.cid.isEmpty()) {
        return false; //filter out related parts
    }
    return true;
}

bool filterSubAttachmentParts(AttachmentPart part)
{
    if (isSubPart<AttachmentPart>(part)) {
        return false; // filter out CertPart f.ex.
    }
    return true;
}

List<T> collect<T>(Part start, std::function<bool(const Part &)> select, std::function<bool(const std::shared_ptr<T> &)> filter) {
    List<T> col;
    if (!select(start)) {
        return col;
    }

    if(isOrSubTypeIs<T>(start) && filter(start.staticCast<T>)){
        col.append(p);
    }
    foreach(childs as child) {
        if (select(child)) {
            col.expand(collect(child,select,filter);
        }
    }
    return col;
}