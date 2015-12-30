/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/

#include "messageviewer/messageviewerutil.h"
#include "kpimtextedit/texttospeech.h"
#include "utils/iconnamecache.h"
#include "messageviewer/nodehelper.h"
#include "messageviewer_debug.h"
#include "MessageCore/MessageCoreSettings"
#include "MessageCore/NodeHelper"
#include "MessageCore/StringUtil"

#include "PimCommon/RenameFileDialog"

#include <AkonadiCore/item.h>

#include <kmbox/mbox.h>

#include <KMime/Message>

#include <kcharsets.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <QFileDialog>
#include <ktoolinvocation.h>
#include <KJobWidgets>
#include <KIO/StatJob>
#include <KIO/FileCopyJob>

#include <QAction>
#include <QIcon>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QTextCodec>
#include <QWidget>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QActionGroup>
#include <QPointer>
#include <QMimeDatabase>

using namespace MessageViewer;

bool Util::checkOverwrite(const QUrl &url, QWidget *w)
{
    bool fileExists = false;
    if (url.isLocalFile()) {
        fileExists = QFile::exists(url.toLocalFile());
    } else {
        auto job = KIO::stat(url, KIO::StatJob::DestinationSide, 0);
        KJobWidgets::setWindow(job, w);
        fileExists = job->exec();
    }
    if (fileExists) {
        if (KMessageBox::Cancel == KMessageBox::warningContinueCancel(
                    w,
                    i18n("A file named \"%1\" already exists. "
                         "Are you sure you want to overwrite it?", url.toDisplayString()),
                    i18n("Overwrite File?"),
                    KStandardGuiItem::overwrite())) {
            return false;
        }
    }
    return true;
}

QString Util::fileNameForMimetype(const QString &mimeType, int iconSize,
                                  const QString &fallbackFileName1,
                                  const QString &fallbackFileName2)
{
    QString fileName;
    QString tMimeType = mimeType;

    // convert non-registered types to registered types
    if (mimeType == QLatin1String("application/x-vnd.kolab.contact")) {
        tMimeType = QStringLiteral("text/x-vcard");
    } else if (mimeType == QLatin1String("application/x-vnd.kolab.event")) {
        tMimeType = QStringLiteral("application/x-vnd.akonadi.calendar.event");
    } else if (mimeType == QLatin1String("application/x-vnd.kolab.task")) {
        tMimeType = QStringLiteral("application/x-vnd.akonadi.calendar.todo");
    } else if (mimeType == QLatin1String("application/x-vnd.kolab.journal")) {
        tMimeType = QStringLiteral("application/x-vnd.akonadi.calendar.journal");
    } else if (mimeType == QLatin1String("application/x-vnd.kolab.note")) {
        tMimeType = QStringLiteral("application/x-vnd.akonadi.note");
    }
    QMimeDatabase mimeDb;
    auto mime = mimeDb.mimeTypeForName(tMimeType);
    if (mime.isValid()) {
        fileName = mime.iconName();
    } else {
        fileName = QStringLiteral("unknown");
        if (!tMimeType.isEmpty()) {
            qCWarning(MESSAGEVIEWER_LOG) << "unknown mimetype" << tMimeType;
        }
    }
    //WorkAround for #199083
    if (fileName == QLatin1String("text-vcard")) {
        fileName = QStringLiteral("text-x-vcard");
    }
    if (fileName.isEmpty()) {
        fileName = fallbackFileName1;
        if (fileName.isEmpty()) {
            fileName = fallbackFileName2;
        }
        if (!fileName.isEmpty()) {
            fileName = mimeDb.mimeTypeForFile(QLatin1String("/tmp/") + fileName).iconName();
        }
    }

    return IconNameCache::instance()->iconPath(fileName, iconSize);
}

#if defined Q_OS_WIN || defined Q_OS_MACX
#include <QDesktopServices>
#endif

bool Util::handleUrlWithQDesktopServices(const QUrl &url)
{
#if defined Q_OS_WIN || defined Q_OS_MACX
    QDesktopServices::openUrl(url);
    return true;
#else
    Q_UNUSED(url);
    return false;
#endif
}

KMime::Content::List Util::allContents(const KMime::Content *message)
{
    KMime::Content::List result;
    KMime::Content *child = MessageCore::NodeHelper::firstChild(message);
    if (child) {
        result += child;
        result += allContents(child);
    }
    KMime::Content *next = MessageCore::NodeHelper::nextSibling(message);
    if (next) {
        result += next;
        result += allContents(next);
    }

    return result;
}

bool Util::saveContents(QWidget *parent, const KMime::Content::List &contents, QUrl &currentFolder)
{
    QUrl url, dirUrl;
    const bool multiple = (contents.count() > 1);
    if (multiple) {
        // get the dir
        dirUrl = QFileDialog::getExistingDirectoryUrl(parent, i18n("Save Attachments To"));
        if (!dirUrl.isValid()) {
            return false;
        }

        // we may not get a slash-terminated url out of KFileDialog
        if (!dirUrl.path().endsWith(QLatin1Char('/'))) {
            dirUrl.setPath(dirUrl.path() + QLatin1Char('/'));
        }
        currentFolder = dirUrl;
    } else {
        // only one item, get the desired filename
        KMime::Content *content = contents.first();
        QString fileName = NodeHelper::fileName(content);
        fileName = MessageCore::StringUtil::cleanFileName(fileName);
        if (fileName.isEmpty()) {
            fileName = i18nc("filename for an unnamed attachment", "attachment.1");
        }
        QUrl pathUrl(QUrl::fromLocalFile(fileName));
        url = QFileDialog::getSaveFileUrl(parent, i18n("Save Attachment"), pathUrl);
        if (url.isEmpty()) {
            return false;
        }
        currentFolder = KIO::upUrl(url);
    }
    QMap< QString, int > renameNumbering;

    bool globalResult = true;
    int unnamedAtmCount = 0;
    PimCommon::RenameFileDialog::RenameFileDialogResult result = PimCommon::RenameFileDialog::RENAMEFILE_IGNORE;
    foreach (KMime::Content *content, contents) {
        QUrl curUrl;
        if (!dirUrl.isEmpty()) {
            curUrl = dirUrl;
            QString fileName = MessageViewer::NodeHelper::fileName(content);
            fileName = MessageCore::StringUtil::cleanFileName(fileName);
            if (fileName.isEmpty()) {
                ++unnamedAtmCount;
                fileName = i18nc("filename for the %1-th unnamed attachment",
                                 "attachment.%1", unnamedAtmCount);
            }
            if (!curUrl.path().endsWith(QLatin1Char('/'))) {
                curUrl.setPath(curUrl.path() + QLatin1Char('/'));
            }
            curUrl.setPath(curUrl.path() + fileName);
        } else {
            curUrl = url;
        }
        if (!curUrl.isEmpty()) {
            //Bug #312954
            if (multiple && (curUrl.fileName() == QLatin1String("smime.p7s"))) {
                continue;
            }
            // Rename the file if we have already saved one with the same name:
            // try appending a number before extension (e.g. "pic.jpg" => "pic_2.jpg")
            QString origFile = curUrl.fileName();
            QString file = origFile;

            while (renameNumbering.contains(file)) {
                file = origFile;
                int num = renameNumbering[file] + 1;
                int dotIdx = file.lastIndexOf(QLatin1Char('.'));
                file = file.insert((dotIdx >= 0) ? dotIdx : file.length(), QStringLiteral("_") + QString::number(num));
            }
            curUrl = curUrl.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);
            curUrl.setPath(curUrl.path() + QLatin1Char('/') + file);

            // Increment the counter for both the old and the new filename
            if (!renameNumbering.contains(origFile)) {
                renameNumbering[origFile] = 1;
            } else {
                renameNumbering[origFile]++;
            }

            if (file != origFile) {
                if (!renameNumbering.contains(file)) {
                    renameNumbering[file] = 1;
                } else {
                    renameNumbering[file]++;
                }
            }

            if (!(result == PimCommon::RenameFileDialog::RENAMEFILE_OVERWRITEALL ||
                    result == PimCommon::RenameFileDialog::RENAMEFILE_IGNOREALL)) {
                bool fileExists = false;
                if (curUrl.isLocalFile()) {
                    fileExists = QFile::exists(curUrl.toLocalFile());
                } else {
                    auto job = KIO::stat(curUrl, KIO::StatJob::DestinationSide, 0);
                    KJobWidgets::setWindow(job, parent);
                    fileExists = job->exec();
                }
                if (fileExists) {
                    PimCommon::RenameFileDialog *dlg = new PimCommon::RenameFileDialog(curUrl, multiple, parent);
                    result = static_cast<PimCommon::RenameFileDialog::RenameFileDialogResult>(dlg->exec());
                    if (result == PimCommon::RenameFileDialog::RENAMEFILE_IGNORE ||
                            result == PimCommon::RenameFileDialog::RENAMEFILE_IGNOREALL) {
                        delete dlg;
                        continue;
                    } else if (result == PimCommon::RenameFileDialog::RENAMEFILE_RENAME) {
                        curUrl = dlg->newName();
                    }
                    delete dlg;
                }
            }
            // save
            if (result != PimCommon::RenameFileDialog::RENAMEFILE_IGNOREALL) {
                const bool result = saveContent(parent, content, curUrl);
                if (!result) {
                    globalResult = result;
                }
            }
        }
    }

    return globalResult;
}

bool Util::saveContent(QWidget *parent, KMime::Content *content, const QUrl &url)
{
    // FIXME: This is all horribly broken. First of all, creating a NodeHelper and then immediatley
    //        reading out the encryption/signature state will not work at all.
    //        Then, topLevel() will not work for attachments that are inside encrypted parts.
    //        What should actually be done is either passing in an ObjectTreeParser that has already
    //        parsed the message, or creating an OTP here (which would have the downside that the
    //        password dialog for decrypting messages is shown twice)
#if 0 // totally broken
    KMime::Content *topContent  = content->topLevel();
    MessageViewer::NodeHelper *mNodeHelper = new MessageViewer::NodeHelper;
    bool bSaveEncrypted = false;
    bool bEncryptedParts = mNodeHelper->encryptionState(content) != MessageViewer::KMMsgNotEncrypted;
    if (bEncryptedParts)
        if (KMessageBox::questionYesNo(parent,
                                       i18n("The part %1 of the message is encrypted. Do you want to keep the encryption when saving?",
                                            url.fileName()),
                                       i18n("KMail Question"), KGuiItem(i18n("Keep Encryption")), KGuiItem(i18n("Do Not Keep"))) ==
                KMessageBox::Yes) {
            bSaveEncrypted = true;
        }

    bool bSaveWithSig = true;
    if (mNodeHelper->signatureState(content) != MessageViewer::KMMsgNotSigned)
        if (KMessageBox::questionYesNo(parent,
                                       i18n("The part %1 of the message is signed. Do you want to keep the signature when saving?",
                                            url.fileName()),
                                       i18n("KMail Question"), KGuiItem(i18n("Keep Signature")), KGuiItem(i18n("Do Not Keep"))) !=
                KMessageBox::Yes) {
            bSaveWithSig = false;
        }

    QByteArray data;
    if (bSaveEncrypted || !bEncryptedParts) {
        KMime::Content *dataNode = content;
        QByteArray rawDecryptedBody;
        bool gotRawDecryptedBody = false;
        if (!bSaveWithSig) {
            if (topContent->contentType()->mimeType() == "multipart/signed")  {
                // carefully look for the part that is *not* the signature part:
                if (ObjectTreeParser::findType(topContent, "application/pgp-signature", true, false)) {
                    dataNode = ObjectTreeParser::findTypeNot(topContent, "application", "pgp-signature", true, false);
                } else if (ObjectTreeParser::findType(topContent, "application/pkcs7-mime", true, false)) {
                    dataNode = ObjectTreeParser::findTypeNot(topContent, "application", "pkcs7-mime", true, false);
                } else {
                    dataNode = ObjectTreeParser::findTypeNot(topContent, "multipart", "", true, false);
                }
            } else {
                EmptySource emptySource;
                ObjectTreeParser otp(&emptySource, 0, 0, false, false);

                // process this node and all it's siblings and descendants
                mNodeHelper->setNodeUnprocessed(dataNode, true);
                otp.parseObjectTree(dataNode);

                rawDecryptedBody = otp.rawDecryptedBody();
                gotRawDecryptedBody = true;
            }
        }
        QByteArray cstr = gotRawDecryptedBody
                          ? rawDecryptedBody
                          : dataNode->decodedContent();
        data = KMime::CRLFtoLF(cstr);
    }
#else
    const QByteArray data = content->decodedContent();
    qCWarning(MESSAGEVIEWER_LOG) << "Port the encryption/signature handling when saving a KMime::Content.";
#endif
    QDataStream ds;
    QFile file;
    QTemporaryFile tf;
    if (url.isLocalFile()) {
        // save directly
        file.setFileName(url.toLocalFile());
        if (!file.open(QIODevice::WriteOnly)) {
            KMessageBox::error(parent,
                               xi18nc("1 = file name, 2 = error string",
                                      "<qt>Could not write to the file<br><filename>%1</filename><br><br>%2",
                                      file.fileName(),
                                      file.errorString()),
                               i18n("Error saving attachment"));
            return false;
        }
        ds.setDevice(&file);
    } else {
        // tmp file for upload
        tf.open();
        ds.setDevice(&tf);
    }

    const int bytesWritten = ds.writeRawData(data.data(), data.size());
    if (bytesWritten != data.size()) {
        QFile *f = static_cast<QFile *>(ds.device());
        KMessageBox::error(parent,
                           xi18nc("1 = file name, 2 = error string",
                                  "<qt>Could not write to the file<br><filename>%1</filename><br><br>%2",
                                  f->fileName(),
                                  f->errorString()),
                           i18n("Error saving attachment"));
        // Remove the newly created empty or partial file
        f->remove();
        return false;
    }

    if (!url.isLocalFile()) {
        // QTemporaryFile::fileName() is only defined while the file is open
        QString tfName = tf.fileName();
        tf.close();
        auto job = KIO::file_copy(QUrl::fromLocalFile(tfName), url);
        KJobWidgets::setWindow(job, parent);
        if (!job->exec()) {
            KMessageBox::error(parent,
                               xi18nc("1 = file name, 2 = error string",
                                      "<qt>Could not write to the file<br><filename>%1</filename><br><br>%2",
                                      url.toDisplayString(),
                                      job->errorString()),
                               i18n("Error saving attachment"));
            return false;
        }
    } else {
        file.close();
    }

#if 0
    mNodeHelper->removeTempFiles();
    delete mNodeHelper;
#endif
    return true;
}

bool Util::saveAttachments(const KMime::Content::List &contents, QWidget *parent, QUrl &currentFolder)
{
    if (contents.isEmpty()) {
        KMessageBox::information(parent, i18n("Found no attachments to save."));
        return false;
    }

    return Util::saveContents(parent, contents, currentFolder);
}

bool Util::saveMessageInMbox(const Akonadi::Item::List &retrievedMsgs, QWidget *parent, bool appendMessages)
{

    QString fileName;
    if (retrievedMsgs.isEmpty()) {
        return true;
    }
    const Akonadi::Item msgBase = retrievedMsgs.first();

    if (msgBase.hasPayload<KMime::Message::Ptr>()) {
        fileName = MessageCore::StringUtil::cleanFileName(MessageViewer::NodeHelper::cleanSubject(msgBase.payload<KMime::Message::Ptr>().data()).trimmed());
        fileName.remove(QLatin1Char('\"'));
    } else {
        fileName = i18n("message");
    }

    if (!fileName.endsWith(QLatin1String(".mbox"))) {
        fileName += QLatin1String(".mbox");
    }

    const QString filter = i18n("email messages (*.mbox);;all files (*)");
    QPointer<QFileDialog> dlg = new QFileDialog(parent, i18np("Save Message", "Save Messages", retrievedMsgs.count()), fileName, filter);
    dlg->setFileMode(QFileDialog::AnyFile);
    dlg->setAcceptMode(QFileDialog::AcceptSave);

    if (appendMessages) {
        dlg->setOption(QFileDialog::DontConfirmOverwrite);
    }
    if (dlg->exec()) {
        QList<QUrl> url = dlg->selectedUrls();
        if (url.isEmpty()) {
            delete dlg;
            return true;
        }

        const QString localFileName = url.at(0).toLocalFile();
        if (localFileName.isEmpty()) {
            delete dlg;
            return true;
        }

        if (!appendMessages) {
            QFile::remove(localFileName);
        }

        KMBox::MBox mbox;
        if (!mbox.load(localFileName)) {
            if (appendMessages) {
                KMessageBox::error(parent, i18n("File %1 could not be loaded.", localFileName), i18n("Error loading message"));
            } else {
                KMessageBox::error(parent, i18n("File %1 could not be created.", localFileName), i18n("Error saving message"));
            }
            delete dlg;
            return false;
        }
        foreach (const Akonadi::Item &item, retrievedMsgs) {
            if (item.hasPayload<KMime::Message::Ptr>()) {
                mbox.appendMessage(item.payload<KMime::Message::Ptr>());
            }
        }

        if (!mbox.save()) {
            KMessageBox::error(parent, i18n("We cannot save message."), i18n("Error saving message"));
            delete dlg;
            return false;
        }
    }
    delete dlg;
    return true;
}

QAction *Util::createAppAction(const KService::Ptr &service, bool singleOffer, QActionGroup *actionGroup, QObject *parent)
{
    QString actionName(service->name().replace(QLatin1Char('&'), QStringLiteral("&&")));
    if (singleOffer) {
        actionName = i18n("Open &with %1", actionName);
    } else {
        actionName = i18nc("@item:inmenu Open With, %1 is application name", "%1", actionName);
    }

    QAction *act = new QAction(parent);
    act->setIcon(QIcon::fromTheme(service->icon()));
    act->setText(actionName);
    actionGroup->addAction(act);
    act->setData(QVariant::fromValue(service));
    return act;
}

QMimeType Util::mimetype(const QString &name)
{
    QMimeDatabase db;
    // consider the filename if mimetype cannot be found by content-type
    auto mimeTypes = db.mimeTypesForFileName(name);
    foreach (const auto &mt, mimeTypes) {
        if (mt.name() != QLatin1String("application/octet-stream")) {
            return mt;
        }
    }

    // consider the attachment's contents if neither the Content-Type header
    // nor the filename give us a clue
    return db.mimeTypeForFile(name);
}
