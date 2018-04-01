/*
    Copyright (c) 2016 Sandro Knauß <knauss@kolabsys.com>
    Copyright (c) 2017 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include "attachmentmodel.h"

#include <mimetreeparser/objecttreeparser.h>
#include <fabric.h>

#include <QDebug>
#include <KMime/Content>
#include <QFile>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QMimeDatabase>

#include <QGpgME/ImportJob>
#include <QGpgME/Protocol>

#include <memory>

QString sizeHuman(float size)
{
    QStringList list;
    list << "KB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("Bytes");

    while(size >= 1024.0 && i.hasNext())
     {
        unit = i.next();
        size /= 1024.0;
    }

    if (unit == "Bytes") {
        return QString().setNum(size) + " " + unit;
    } else {
        return QString().setNum(size,'f',2)+" "+unit;
    }
}

class AttachmentModelPrivate
{
public:
    AttachmentModelPrivate(AttachmentModel *q_ptr, const std::shared_ptr<MimeTreeParser::ObjectTreeParser> &parser);

    AttachmentModel *q;
    std::shared_ptr<MimeTreeParser::ObjectTreeParser> mParser;
    QVector<MimeTreeParser::MessagePartPtr> mAttachments;
};

AttachmentModelPrivate::AttachmentModelPrivate(AttachmentModel* q_ptr, const std::shared_ptr<MimeTreeParser::ObjectTreeParser>& parser)
    : q(q_ptr)
    , mParser(parser)
{
    mAttachments = mParser->collectAttachmentParts();
}

AttachmentModel::AttachmentModel(std::shared_ptr<MimeTreeParser::ObjectTreeParser> parser)
    : d(std::unique_ptr<AttachmentModelPrivate>(new AttachmentModelPrivate(this, parser)))
{
}

AttachmentModel::~AttachmentModel()
{
}

QHash<int, QByteArray> AttachmentModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[NameRole] = "name";
    roles[SizeRole] = "size";
    roles[IconRole] = "iconName";
    roles[IsEncryptedRole] = "encrypted";
    roles[IsSignedRole] = "signed";
    return roles;
}

QModelIndex AttachmentModel::index(int row, int column, const QModelIndex &) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }

    if (row < d->mAttachments.size()) {
        return createIndex(row, column, d->mAttachments.at(row).data());
    }
    return QModelIndex();
}

QVariant AttachmentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        switch (role) {
        case Qt::DisplayRole:
            return QString("root");
        }
        return QVariant();
    }

    if (index.internalPointer()) {
        const auto part = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
        Q_ASSERT(part);
        auto node = part->node();
        if (!node) {
            qWarning() << "no content for attachment";
            return {};
        }
        QMimeDatabase mimeDb;
        const auto mimetype = mimeDb.mimeTypeForName(QString::fromLatin1(part->mimeType()));
        const auto content = node->encodedContent();
        switch(role) {
        case TypeRole:
            return mimetype.name();
        case NameRole:
            return part->filename();
        case IconRole:
            return mimetype.iconName();
        case SizeRole:
            return sizeHuman(content.size());
        case IsEncryptedRole:
            return part->encryptions().size() > 0;
        case IsSignedRole:
            return part->signatures().size() > 0;
        }
    }
    return QVariant();
}

static QString saveAttachmentToDisk(const QModelIndex &index, const QString &path, bool readonly = false)
{
    if (index.internalPointer()) {
        const auto part = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
        Q_ASSERT(part);
        auto node = part->node();
        auto data = node->decodedContent();
        if (part->isText()) {
            // convert CRLF to LF before writing text attachments to disk
            data = KMime::CRLFtoLF(data);
        }
        auto fname = path + part->filename();
        //A file with that name already exists, we assume it's the right file
        if (QFileInfo{fname}.exists()) {
            return fname;
        }
        QFile f(fname);
        if (!f.open(QIODevice::ReadWrite)) {
            qWarning() << "Failed to write attachment to file:" << fname << " Error: " << f.errorString();
            Kube::Fabric::Fabric{}.postMessage("notification", {{"message", QObject::tr("Failed to save attachment.")}});
            return {};
        }
        f.write(data);
        if (readonly) {
            // make file read-only so that nobody gets the impression that he migh edit attached files
            f.setPermissions(QFileDevice::ReadUser);
        }
        f.close();
        qInfo() << "Wrote attachment to file: " << fname;
        return fname;
    }
    return {};

}

bool AttachmentModel::saveAttachmentToDisk(const QModelIndex &index)
{
    auto downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (downloadDir.isEmpty()) {
        downloadDir = "~";
    }
    downloadDir += "/kube/";
    QDir{}.mkpath(downloadDir);

    auto path = ::saveAttachmentToDisk(index, downloadDir);
    if (path.isEmpty()) {
        return false;
    }
    Kube::Fabric::Fabric{}.postMessage("notification", {{"message", tr("Saved the attachment to disk: ") + path}});
    return true;
}

bool AttachmentModel::openAttachment(const QModelIndex &index)
{
    auto downloadDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)+ "/kube/";
    QDir{}.mkpath(downloadDir);
    const auto filePath = ::saveAttachmentToDisk(index, downloadDir, true);
    if (!filePath.isEmpty()) {
        if (!QDesktopServices::openUrl(QUrl("file://" + filePath))) {
            Kube::Fabric::Fabric{}.postMessage("notification", {{"message", tr("Failed to open attachment.")}});
            return false;
        }
        return true;
    }
    Kube::Fabric::Fabric{}.postMessage("notification", {{"message", tr("Failed to save attachment for opening.")}});
    return false;
}

bool AttachmentModel::importPublicKey(const QModelIndex &index)
{
    Q_ASSERT(index.internalPointer());
    const auto part = static_cast<MimeTreeParser::MessagePart *>(index.internalPointer());
    Q_ASSERT(part);
    auto pkey = part->node()->decodedContent();

    const auto *proto = QGpgME::openpgp();
    std::unique_ptr<QGpgME::ImportJob> job(proto->importJob());
    auto result = job->exec(pkey);

    bool success = true;

    QString message;
    if(result.numConsidered() == 0) {
        message = tr("No keys were found in this attachment");
        success = false;
    } else {
        message = tr("%n Key(s) imported", "", result.numImported());
        if(result.numUnchanged() != 0) {
            message += "\n" + tr("%n Key(s) were already imported", "", result.numUnchanged());
        }
    }

    Kube::Fabric::Fabric{}.postMessage("notification",
        {{"message", message}});

    return success;
}

QModelIndex AttachmentModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int AttachmentModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->mAttachments.size();
    }
    return 0;
}

int AttachmentModel::columnCount(const QModelIndex &) const
{
    return 1;
}
