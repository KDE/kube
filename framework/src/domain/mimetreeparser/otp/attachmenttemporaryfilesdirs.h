/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

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

#ifndef ATTACHMENTTEMPORARYFILESDIRS_H
#define ATTACHMENTTEMPORARYFILESDIRS_H

#include <QObject>
#include <QStringList>

namespace MimeTreeParser
{
class AttachmentTemporaryFilesDirsPrivate;

class AttachmentTemporaryFilesDirs : public QObject
{
    Q_OBJECT
public:
    explicit AttachmentTemporaryFilesDirs(QObject *parent = nullptr);
    ~AttachmentTemporaryFilesDirs();

    void addTempFile(const QString &file);
    void addTempDir(const QString &dir);
    QStringList temporaryFiles() const;
    void removeTempFiles();
    void forceCleanTempFiles();

    QStringList temporaryDirs() const;

    void setDelayRemoveAllInMs(int ms);

private Q_SLOTS:
    void slotRemoveTempFiles();

private:
    AttachmentTemporaryFilesDirsPrivate *const d;
};

}

#endif // ATTACHMENTTEMPORARYFILESDIRS_H
