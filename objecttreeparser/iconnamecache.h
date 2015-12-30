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
#ifndef ICONNAMECACHE_H
#define ICONNAMECACHE_H

#include <QMap>
#include <QString>

namespace MessageViewer
{

/**
 * This class is a replacement for KIconLoader::iconPath(), because the iconPath()
 * function can be slow for non-existing icons or icons that fall back to a generic icon.
 * Reason is that KIconLoader does slow system calls for finding the icons.
 *
 * The IconNameCache caches the result of iconPath() in a map and solves the slowness.
 */
class IconNameCache
{
public:

    static IconNameCache *instance();
    QString iconPath(const QString &name, int size) const;

private:

    class Entry
    {
    public:
        QString fileName;
        int size;

        bool operator < (const Entry &other) const;
    };

    mutable QMap<Entry, QString> mCachedEntries;
};

}
#endif
