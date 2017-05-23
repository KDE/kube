/*
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef __MIMETREEPARSER_NODEHELPER_H__
#define __MIMETREEPARSER_NODEHELPER_H__

#include "partmetadata.h"
#include "enums.h"

#include <KMime/Message>

#include <QList>
#include <QMap>
#include <QSet>

class QUrl;
class QTextCodec;

namespace MimeTreeParser
{
class AttachmentTemporaryFilesDirs;
namespace Interface
{
class BodyPartMemento;
}
}

namespace MimeTreeParser
{

/**
 * @author Andras Mantia <andras@kdab.net>
 */
class NodeHelper: public QObject
{
    Q_OBJECT
public:
    NodeHelper();

    ~NodeHelper();

    void setNodeProcessed(KMime::Content *node, bool recurse);
    void setNodeUnprocessed(KMime::Content *node, bool recurse);
    bool nodeProcessed(KMime::Content *node) const;
    void clear();
    void forceCleanTempFiles();

    void setEncryptionState(const KMime::Content *node, const KMMsgEncryptionState state);
    KMMsgEncryptionState encryptionState(const KMime::Content *node) const;

    void setSignatureState(const KMime::Content *node, const KMMsgSignatureState state);
    KMMsgSignatureState signatureState(const KMime::Content *node) const;

    KMMsgSignatureState overallSignatureState(KMime::Content *node) const;
    KMMsgEncryptionState overallEncryptionState(KMime::Content *node) const;

    void setPartMetaData(KMime::Content *node, const PartMetaData &metaData);
    PartMetaData partMetaData(KMime::Content *node);

    /**
     *  Set the 'Content-Type' by mime-magic from the contents of the body.
     *  If autoDecode is true the decoded body will be used for mime type
     *  determination (this does not change the body itself).
     */
    void magicSetType(KMime::Content *node, bool autoDecode = true);

    /** Attach an extra node to an existing node */
    void attachExtraContent(KMime::Content *topLevelNode, KMime::Content *content);

    /** Get the extra nodes attached to the @param topLevelNode and all sub-nodes of @param topLevelNode */
    QList<KMime::Content *> extraContents(KMime::Content *topLevelNode) const;

    /** Return a modified message (node tree) starting from @param topLevelNode that has the original nodes and the extra nodes.
        The caller has the responsibility to delete the new message.
     */
    KMime::Message *messageWithExtraContent(KMime::Content *topLevelNode);

    /** Get a QTextCodec suitable for this message part */
    const QTextCodec *codec(KMime::Content *node);

    /** Set the charset the user selected for the message to display */
    void setOverrideCodec(KMime::Content *node, const QTextCodec *codec);

    Interface::BodyPartMemento *bodyPartMemento(KMime::Content *node, const QByteArray &which) const;

    void setBodyPartMemento(KMime::Content *node, const QByteArray &which,
                            Interface::BodyPartMemento *memento);

    // A flag to remember if the node was embedded. This is useful for attachment nodes, the reader
    // needs to know if they were displayed inline or not.
    bool isNodeDisplayedEmbedded(KMime::Content *node) const;
    void setNodeDisplayedEmbedded(KMime::Content *node, bool displayedEmbedded);

    // Same as above, but this time determines if the node was hidden or not
    bool isNodeDisplayedHidden(KMime::Content *node) const;
    void setNodeDisplayedHidden(KMime::Content *node, bool displayedHidden);

    /**
     * Writes the given message part to a temporary file and returns the
     * name of this file or QString() if writing failed.
     */
    QString writeNodeToTempFile(KMime::Content *node);

    /**
     * Returns the temporary file path and name where this node was saved, or an empty url
     * if it wasn't saved yet with writeNodeToTempFile()
     */
    QUrl tempFileUrlFromNode(const KMime::Content *node);

    /**
     * Creates a temporary dir for saving attachments, etc.
     * Will be automatically deleted when another message is viewed.
     * @param param Optional part of the directory name.
     */
    QString createTempDir(const QString &param = QString());

    /**
     * Cleanup the attachment temp files
     */
    void removeTempFiles();

    /**
     * Add a file to the list of managed temporary files
     */
    void addTempFile(const QString &file);

    // Get a href in the form attachment:<nodeId>?place=<place>, used by ObjectTreeParser and
    // UrlHandlerManager.
    QString asHREF(const KMime::Content *node, const QString &place) const;
    KMime::Content *fromHREF(const KMime::Message::Ptr &mMessage, const QUrl &href) const;

    /**
     * @return true if this node is a child or an encapsulated message
     */
    static bool isInEncapsulatedMessage(KMime::Content *node);

    /**
     * Returns the charset for the given node. If no charset is specified
     * for the node, the defaultCharset() is returned.
     */
    static QByteArray charset(KMime::Content *node);

    /**
     * Return a QTextCodec for the specified charset.
     * This function is a bit more tolerant, than QTextCodec::codecForName
     */
    static const QTextCodec *codecForName(const QByteArray &_str);

    /**
     * Returns a usable filename for a node, that can be the filename from the
     * content disposition header, or if that one is empty, the name from the
     * content type header.
     */
    static QString fileName(const KMime::Content *node);

    /**
     * Fixes an encoding received by a KDE function and returns the proper,
     * MIME-compilant encoding name instead.
     * @see encodingForName
     */
    static QString fixEncoding(const QString &encoding);   //TODO(Andras) move to a utility class?

    /**
     * Drop-in replacement for KCharsets::encodingForName(). The problem with
     * the KCharsets function is that it returns "human-readable" encoding names
     * like "ISO 8859-15" instead of valid encoding names like "ISO-8859-15".
     * This function fixes this by replacing whitespace with a hyphen.
     */
    static QString encodingForName(const QString &descriptiveName);   //TODO(Andras) move to a utility class?

    /**
     * Return a list of the supported encodings
     * @param usAscii if true, US-Ascii encoding will be prepended to the list.
     */
    static QStringList supportedEncodings(bool usAscii);   //TODO(Andras) move to a utility class?

    QString fromAsString(KMime::Content *node) const;

    KMime::Content *decryptedNodeForContent(KMime::Content *content) const;

    /**
     * This function returns the unencrypted message that is based on @p originalMessage.
     * All encrypted MIME parts are removed and replaced by their decrypted plain-text versions.
     * Encrypted parts that are within signed parts are not replaced, since that would invalidate
     * the signature.
     *
     * This only works if the message was run through ObjectTreeParser::parseObjectTree() with the
     * currrent NodeHelper before, because parseObjectTree() actually decrypts the message and stores
     * the decrypted nodes by calling attachExtraContent().
     *
     * @return the unencrypted message or an invalid pointer if the original message didn't contain
     *         a part that needed to be modified.
     */
    KMime::Message::Ptr unencryptedMessage(const KMime::Message::Ptr &originalMessage);

    /**
     * Returns a list of attachments of attached extra content nodes.
     * This is mainly useful is order to get attachments of encrypted messages.
     * Note that this does not include attachments from the primary node tree.
     * @see KMime::Content::attachments().
     */
    QVector<KMime::Content *> attachmentsOfExtraContents() const;

Q_SIGNALS:
    void update(MimeTreeParser::UpdateMode);

private:
    Q_DISABLE_COPY(NodeHelper)
    bool unencryptedMessage_helper(KMime::Content *node, QByteArray &resultingData, bool addHeaders,
                                   int recursionLevel = 1);

    void mergeExtraNodes(KMime::Content *node);
    void cleanFromExtraNodes(KMime::Content *node);

    /** Creates a persistent index string that bridges the gap between the
        permanent nodes and the temporary ones.

        Used internally for robust indexing.
    **/
    QString persistentIndex(const KMime::Content *node) const;

    /** Translates the persistentIndex into a node back

        node: any node of the actually message to what the persistentIndex is interpreded
    **/
    KMime::Content *contentFromIndex(KMime::Content *node, const QString &persistentIndex) const;

private:
    QList<KMime::Content *> mProcessedNodes;
    QList<KMime::Content *> mNodesUnderProcess;
    QMap<const KMime::Content *, KMMsgEncryptionState> mEncryptionState;
    QMap<const KMime::Content *, KMMsgSignatureState> mSignatureState;
    QSet<KMime::Content *> mDisplayEmbeddedNodes;
    QSet<KMime::Content *> mDisplayHiddenNodes;
    QTextCodec *mLocalCodec;
    QMap<KMime::Content *, const QTextCodec *> mOverrideCodecs;
    QMap<QString, QMap<QByteArray, Interface::BodyPartMemento *> > mBodyPartMementoMap;
    QMap<KMime::Content *, PartMetaData> mPartMetaDatas;
    QMap<KMime::Message::Content *, QList<KMime::Content *> > mExtraContents;
    AttachmentTemporaryFilesDirs *mAttachmentFilesDir;

    friend class NodeHelperTest;
};

}

#endif
