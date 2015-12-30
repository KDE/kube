/* Copyright 2009 Thomas McGuire <mcguire@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "iconnamecache.h"

#include <KIconLoader>

namespace MessageViewer
{
Q_GLOBAL_STATIC(IconNameCache, s_iconNameCache)

IconNameCache *IconNameCache::instance()
{
    return s_iconNameCache;
}

bool IconNameCache::Entry::operator < (const Entry &other) const
{
    const int fileNameCompare = fileName.compare(other.fileName);
    if (fileNameCompare != 0) {
        return fileNameCompare < 0;
    } else {
        return size < other.size;
    }
}

QString IconNameCache::iconPath(const QString &name, int size) const
{
    Entry entry;
    entry.fileName = name;
    entry.size = size;

    if (mCachedEntries.contains(entry)) {
        return mCachedEntries.value(entry);
    }

    const QString fileName = KIconLoader::global()->iconPath(name, size);
    mCachedEntries.insert(entry, fileName);
    return fileName;
}
}
