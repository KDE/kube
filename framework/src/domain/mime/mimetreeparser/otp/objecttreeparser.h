/*
    objecttreeparser.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003      Marc Mutz <mutz@kde.org>
    Copyright (C) 2002-2003, 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Copyright (c) 2009 Andras Mantia <andras@kdab.net>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __MIMETREEPARSER_OBJECTTREEPARSER_H__
#define __MIMETREEPARSER_OBJECTTREEPARSER_H__

#include "nodehelper.h"
#include "objecttreesource.h"
#include "messagepart.h"

#include <gpgme++/verificationresult.h>

class QString;

namespace KMime
{
class Content;
}

namespace MimeTreeParser
{

class PartMetaData;
class ViewerPrivate;
class NodeHelper;
class MimeMessagePart;

typedef QSharedPointer<MessagePart> MessagePartPtr;
typedef QSharedPointer<MimeMessagePart> MimeMessagePartPtr;

class ProcessResult
{
public:
    explicit ProcessResult(NodeHelper *nodeHelper, KMMsgSignatureState  inlineSignatureState  = KMMsgNotSigned,
                           KMMsgEncryptionState inlineEncryptionState = KMMsgNotEncrypted,
                           bool neverDisplayInline = false,
                           bool isImage = false)
        : mInlineSignatureState(inlineSignatureState),
          mInlineEncryptionState(inlineEncryptionState),
          mIsImage(isImage),
          mNodeHelper(nodeHelper) {}

    KMMsgSignatureState inlineSignatureState() const;
    void setInlineSignatureState(KMMsgSignatureState state);

    KMMsgEncryptionState inlineEncryptionState() const;
    void setInlineEncryptionState(KMMsgEncryptionState state);


    bool isImage() const;
    void setIsImage(bool image);

    void adjustCryptoStatesOfNode(const KMime::Content *node) const;

private:
    KMMsgSignatureState mInlineSignatureState;
    KMMsgEncryptionState mInlineEncryptionState;
    bool mNeverDisplayInline : 1;
    bool mIsImage : 1;
    NodeHelper *mNodeHelper;
};

/**
\brief Parses messages and generates HTML display code out of them

\par Introduction

First, have a look at the documentation in Mainpage.dox and at the documentation of ViewerPrivate
to understand the broader picture.

Just a note on the terminology: 'Node' refers to a MIME part here, which in KMime is a
KMime::Content.

\par Basics

The ObjectTreeParser basically has two modes: Generating the HTML code for the Viewer, or only
extracting the plainTextContent() for situations where only the message text is needed, for example
when inline forwarding a message. The mode depends on the Interface::ObjectTreeSource passed to the
constructor: If Interface::ObjectTreeSource::htmlWriter() is not 0, then the HTML code generation mode is
used.

Basically, all the ObjectTreeParser does is going through the tree of MIME parts and operating on
those nodes. Operating here means creating the HTML code for the node or extracting the textual
content from it. This process is started with parseObjectTree(), where we loop over the subnodes
of the current root node. For each of those subnodes, we try to find a BodyPartFormatter that can
handle the type of the node. This can either be an internal function, such as
processMultiPartAlternativeSubtype() or processTextHtmlSubtype(), or it can be an external plugin.
More on external plugins later. When no matching formatter is found, defaultHandling() is called
for that node.

\par Multipart Nodes

Those nodes that are of type multipart have subnodes. If one of those children needs to be
processed normally, the processMultipartXXX() functions call stdChildHandling() for the node that
should be handled normally. stdChildHandling() creates its own ObjectTreeParser, which is a clone
of the current ObjectTreeParser, and processes the node. stdChildHandling() is not called for all
children of the multipart node, for example processMultiPartAlternativeSubtype() only calls it on
one of the children, as the other one doesn't need to be displayed. Similary,
processMultiPartSignedSubtype() doesn't call stdChildHandling() for the signature node, only for the
signed node.

\par Processed and Unprocessed Nodes

When a BodyPartFormatter has finished processing a node, it is processed. Nodes are set to being
not processed at the beginning of parseObjectTree(). The processed state of a node is saved in a
list in NodeHelper, see NodeHelper::setNodeProcessed(), NodeHelper::nodeProcessed() and the other
related helper functions.

It is the responsibility of the BodyPartFormatter to correctly call setNodeProcessed() and the
related functions. This is important so that processing the same node twice can be prevented. The
check that prevents duplicate processing is in parseObjectTree().

An example where duplicate processing would happen if we didn't check for it is in stdChildHandling(),
which is for example called from processMultiPartAlternativeSubtype(). Let's say the setting is to
prefer HTML over plain text. In this case, processMultiPartAlternativeSubtype() would call
stdChildHandling() on the HTML node, which would create a new ObjectTreeParser and call
parseObjectTree() on it. parseObjectTree() processes the node and all its siblings, and one of the
siblings is the plain text node, which shouldn't be processed! Therefore
processMultiPartAlternativeSubtype() sets the plain text node as been processed already.

\par Plain Text Output

Various nodes have plain text that should be displayed. This plain text is usually processed though
writeBodyString() first. That method checks if the provided text is an inline PGP text and decrypts
it if necessary. It also pushes the text through quotedHTML(), which does a number of things like
coloring quoted lines or detecting links and creating real link tags for them.

\par Modifying the Message

The ObjectTreeParser does not only parse its message, in some circumstances it also modifies it
before displaying. This is for example the case when displaying a decrypted message: The original
message only contains a binary blob of crypto data, and processMultiPartEncryptedSubtype() decrypts
that blob. After decryption, the current node is replaced with the decrypted node, which happens
in insertAndParseNewChildNode().

\par Crypto Operations

For signature and decryption handling, there are functions which help with generating the HTML code
for the signature header and footer. These are writeDeferredDecryptionBlock(), writeSigstatFooter()
and writeSigstatHeader(). As the name writeDeferredDecryptionBlock() suggests, a setting can cause
the message to not be decrypted unless the user clicks a link. Whether the message should be
decrypted or not can be controlled by Interface::ObjectTreeSource::decryptMessage(). When the user clicks the
decryption link, the URLHandler for 'kmail:' URLs sets that variable to true and triggers an update
of the Viewer, which will cause parseObjectTree() to be called again.

\par Async Crypto Operations

The above case describes decryption the message in place. However, decryption and also verifying of
the signature can take a long time, so synchronous decryption and verifing would cause the Viewer to
block. Therefore it is possible to run these operations in async mode, see allowAsync().
In the first run of the async mode, all the ObjectTreeParser does is starting the decrypt or the
verify job, and informing the user that the operation is in progress with
writeDecryptionInProgressBlock() or with writeSigstatHeader(). Then, it creates and associates a
BodyPartMemento with the current node, for example a VerifyDetachedBodyPartMemento. Each node can
have multiple mementos associated with it, which are differeniated by name.

NodeHelper::setBodyPartMemento() and NodeHelper::bodyPartMemento() provide means to store and
retrieve these mementos. A memento is basically a thin wrapper around the crypto job, it stores the
job pointer, the job input data and the job result. Mementos can be used for any async situation,
not just for crypto jobs, but I'll describe crypto jobs here.

So in the first run of decrypting or verifying a message, the BodyPartFormatter only starts the
crypto job, creates the BodyPartMemento and writes the HTML code that tells the user that the
operation is in progress. parseObjectTree() thus finishes without waiting for anything, and the
message is displayed.

At some point, the crypto jobs then finish, which will cause slotResult() of the BodyPartMemento
to be called. slotResult() then saves the result to some member variable and calls
BodyPartMemento::notify(), which in the end will trigger an update of the Viewer. That update
will, in ViewerPrivate::parseMsg(), create a new ObjectTreeParser and call parseObjectTree() on it.
This is where the second run begins.

The functions that deal with decrypting of verifying, like processMultiPartSignedSubtype() or
processMultiPartEncryptedSubtype() will look if they find a BodyPartMemento that is associated with
the current node. Now it finds that memento, since it was created in the first run. It checks if the
memento's job has finished, and if so, the result can be written out (either the decrypted data or
the verified signature).

When dealing with encrypted nodes, new nodes are created with the decrypted data. It is important to
note that the original MIME tree is never modified, and remains the same as the original one. The method
createAndParseTempNode is called with the newly decrypted data, and it generates a new temporary node to
store the decrypted data. When these nodes are created, it is important to keep track of them as otherwise
some mementos that are added to the newly created temporary nodes will be constantly regenerated. As the
regeneration triggers a viewer update when complete, it results in an infinite refresh loop. The function
NodeHelper::linkAsPermanentDecrypted will create a link between the newly created node and the original parent.
Conversely, the function NodeHelper::attachExtraContent will create a link in the other direction, from the parent
node to the newly created temporary node.

When generating some mementos for nodes that may be temporary nodes (for example, contact photo mementos), the
function NodeHelper::setBodyPartMementoForPermanentParent is used. This will save the given body part memento for
the closest found permanent parent node, rather than the transient node itself. Then when checking for the existence
of a certain memento in a node, NodeHelper::findPermanentParentBodyPartMemento will check to see if any parent of the
given temporary node is a permanent (encrypted) node that has been used to generate the asked-for node.

To conclude: For async operations, parseObjectTree() is called twice: The first call starts the
crypto operation and creates the BodyPartMemento, the second calls sees that the BodyPartMemento is
there and can use its result for writing out the HTML.

\par PartMetaData and ProcessResult

For crypto operations, the class PartMetaData is used a lot, mainly to pass around info about the
crypto state of a node. A PartMetaData can also be associated with a node by using
NodeHelper::setPartMetaData(). The only user of that however is MessageAnalyzer::processPart() of
the Nepomuk E-Mail Feeder, which also uses the ObjectTreeParser to analyze the message.

You'll notice that a ProcessResult is passed to each formatter. The formatter is supposed to modify
the ProcessResult to tell the callers something about the state of the nodes that were processed.
One example for its use is to tell the caller about the crypto state of the node.

\par BodyPartFormatter Plugins

As mentioned way earlier, BodyPartFormatter can either be plugins or be internal. bodypartformatter.cpp
contains some trickery so that the processXXX() methods of the ObjectTreeParser are called from
a BodyPartFormatter associated with them, see the CREATE_BODY_PART_FORMATTER macro.

The BodyPartFormatter code is work in progress, it was supposed to be refactored, but that has not
yet happened at the time of writing. Therefore the code can seem a bit chaotic.

External plugins are loaded with loadPlugins() in bodypartformatterfactory.cpp. External plugins
can only use the classes in the interfaces/ directory, they include BodyPart, BodyPartMemento,
BodyPartFormatterPlugin, BodyPartFormatter, BodyPartURLHandler, HtmlWriter and URLHandler. Therefore
external plugins have powerful capabilities, which are needed for example in the iCal formatter or
in the vCard formatter.

\par Special HTML tags

As also mentioned in the documentation of ViewerPrivate, the ObjectTreeParser writes out special
links that are only understood by the viewer, for example 'kmail:' URLs or 'attachment:' URLs.
Also, some special HTML tags are created, which the Viewer later uses for post-processing. For
example a div with the id 'attachmentInjectionPoint', or a div with the id 'attachmentDiv', which
is used to mark an attachment in the body with a yellow border when the user clicks the attachment
in the header. Finally, parseObjectTree() creates an anchor with the id 'att%1', which is used in
the Viewer to scroll to the attachment.
*/
class ObjectTreeParser
{
    //Disable copy
    ObjectTreeParser(const ObjectTreeParser &other);
public:
    explicit ObjectTreeParser();

    explicit ObjectTreeParser(Interface::ObjectTreeSource *source,
                              NodeHelper *nodeHelper = nullptr,
                              bool showOneMimePart = false
                              );

    virtual ~ObjectTreeParser();

    void setAllowAsync(bool allow);
    bool allowAsync() const;

    bool hasPendingAsyncJobs() const;
    void print();

    /**
    * The text of the message, ie. what would appear in the
    * composer's text editor if this was edited or replied to.
    * This is usually the content of the first text/plain MIME part.
    */
    QString plainTextContent() const;

    /**
    * Similar to plainTextContent(), but returns the HTML source of the first text/html MIME part.
    *
    * Not to be consfused with the HTML code that the message viewer widget displays, that HTML
    * is written out by htmlWriter() and a totally different pair of shoes.
    */
    QString htmlContent() const;

    /**
    * The original charset of MIME part the plain text was extracted from.
    *
    * If there were more than one text/plain MIME parts in the mail, the this is the charset
    * of the last MIME part processed.
    */
    QByteArray plainTextContentCharset() const;
    QByteArray htmlContentCharset() const;

    bool showOnlyOneMimePart() const;
    void setShowOnlyOneMimePart(bool show);

    NodeHelper *nodeHelper() const;

    /** Parse beginning at a given node and recursively parsing
      the children of that node and it's next sibling. */
    void parseObjectTree(KMime::Content *node);
    void parseObjectTree(const QByteArray &mimeMessage);
    MessagePartPtr parsedPart() const;
    KMime::Content *find(const std::function<bool(KMime::Content *)> &select);
    QVector<MessagePartPtr> collectContentParts();
    QVector<MessagePartPtr> collectAttachmentParts();
    void decryptParts();

    /** Embedd content referenced by cid by inlining */
    QString resolveCidLinks(const QString &html);

private:
    /**
    * Does the actual work for parseObjectTree. Unlike parseObjectTree(), this does not change the
    * top-level content.
    */
    MessagePartPtr parseObjectTreeInternal(KMime::Content *node, bool mOnlyOneMimePart);
    MessagePartPtr processType(KMime::Content *node, MimeTreeParser::ProcessResult &processResult, const QByteArray &mediaType, const QByteArray &subType, bool onlyOneMimePart);

    MessagePartPtr defaultHandling(KMime::Content *node, MimeTreeParser::ProcessResult &result, bool onlyOneMimePart);

private:

    /** ctor helper */
    void init();

    const QTextCodec *codecFor(KMime::Content *node) const;
private:
    Interface::ObjectTreeSource *mSource;
    NodeHelper *mNodeHelper;
    QByteArray mPlainTextContentCharset;
    QByteArray mHtmlContentCharset;
    QString mPlainTextContent;
    QString mHtmlContent;
    KMime::Content *mTopLevelContent;
    MessagePartPtr mParsedPart;

    /// Show only one mime part means that the user has selected some node in the message structure
    /// viewer that is not the root, which means the user wants to only see the selected node and its
    /// children. If that is the case, this variable is set to true.
    /// The code needs to behave differently if this is set. For example, it should not process the
    /// siblings. Also, consider inline images: Normally, those nodes are completely hidden, as the
    /// HTML node embedds them. However, when showing only the node of the image, one has to show them,
    /// as their is no HTML node in which they are displayed. There are many more cases where this
    /// variable needs to be obeyed.
    /// This variable is set to false again when processing the children in stdChildHandling(), as
    /// the children can be completely displayed again.
    bool mShowOnlyOneMimePart;

    bool mHasPendingAsyncJobs;
    bool mAllowAsync;
    bool mDeleteNodeHelper;
    KMime::Message::Ptr mMsg;

    friend class PartNodeBodyPart;
    friend class MessagePart;
    friend class EncryptedMessagePart;
    friend class SignedMessagePart;
    friend class EncapsulatedRfc822MessagePart;
    friend class TextMessagePart;
    friend class HtmlMessagePart;
    friend class TextPlainBodyPartFormatter;
    friend class MultiPartSignedBodyPartFormatter;
    friend class ApplicationPkcs7MimeBodyPartFormatter;
};

}

#endif // __MIMETREEPARSER_OBJECTTREEPARSER_H__

