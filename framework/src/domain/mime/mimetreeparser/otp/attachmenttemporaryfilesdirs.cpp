/*
  Copyright (c) 2013-2017 Montel Laurent <montel@kde.org>

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

#include "attachmenttemporaryfilesdirs.h"

#include <QDir>
#include <QFile>
#include <QTimer>

using namespace MimeTreeParser;

class MimeTreeParser::AttachmentTemporaryFilesDirsPrivate
{
public:
    AttachmentTemporaryFilesDirsPrivate()
        : mDelayRemoveAll(10000)
    {

    }
    QStringList mTempFiles;
    QStringList mTempDirs;
    int mDelayRemoveAll;
};

AttachmentTemporaryFilesDirs::AttachmentTemporaryFilesDirs(QObject *parent)
    : QObject(parent),
      d(new AttachmentTemporaryFilesDirsPrivate)
{

}

AttachmentTemporaryFilesDirs::~AttachmentTemporaryFilesDirs()
{
    delete d;
}

void AttachmentTemporaryFilesDirs::setDelayRemoveAllInMs(int ms)
{
    d->mDelayRemoveAll = (ms < 0) ? 0 : ms;
}

void AttachmentTemporaryFilesDirs::removeTempFiles()
{
    QTimer::singleShot(d->mDelayRemoveAll, this, &AttachmentTemporaryFilesDirs::slotRemoveTempFiles);
}

void AttachmentTemporaryFilesDirs::forceCleanTempFiles()
{
    QStringList::ConstIterator end = d->mTempFiles.constEnd();
    for (QStringList::ConstIterator it = d->mTempFiles.constBegin(); it != end; ++it) {
        QFile::remove(*it);
    }
    d->mTempFiles.clear();
    end = d->mTempDirs.constEnd();
    for (QStringList::ConstIterator it = d->mTempDirs.constBegin(); it != end; ++it) {
        QDir(*it).rmdir(*it);
    }
    d->mTempDirs.clear();
}

void AttachmentTemporaryFilesDirs::slotRemoveTempFiles()
{
    forceCleanTempFiles();
    //Delete it after cleaning
    deleteLater();
}

void AttachmentTemporaryFilesDirs::addTempFile(const QString &file)
{
    if (!d->mTempFiles.contains(file)) {
        d->mTempFiles.append(file);
    }
}

void AttachmentTemporaryFilesDirs::addTempDir(const QString &dir)
{
    if (!d->mTempDirs.contains(dir)) {
        d->mTempDirs.append(dir);
    }
}

QStringList AttachmentTemporaryFilesDirs::temporaryFiles() const
{
    return d->mTempFiles;
}

QStringList AttachmentTemporaryFilesDirs::temporaryDirs() const
{
    return d->mTempDirs;
}

