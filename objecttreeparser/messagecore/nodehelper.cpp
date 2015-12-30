/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

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

#include "nodehelper.h"

#include <kmime/kmime_content.h>
#include <kmime/kmime_message.h>

KMime::Content *MessageCore::NodeHelper::nextSibling(const KMime::Content *node)
{
    if (!node) {
        return 0;
    }

    KMime::Content *next = 0;
    KMime::Content *parent = node->parent();
    if (parent) {
        const auto contents = parent->contents();
        const int index = contents.indexOf(const_cast<KMime::Content *>(node)) + 1;
        if (index < contents.size()) { //next on the same level
            next =  contents.at(index);
        }
    }

    return next;
}

KMime::Content *MessageCore::NodeHelper::next(KMime::Content *node, bool allowChildren)
{
    if (allowChildren) {
        if (KMime::Content *child = firstChild(node)) {
            return child;
        }
    }

    if (KMime::Content *sibling = nextSibling(node)) {
        return sibling;
    }

    for (KMime::Content *parent = node->parent(); parent;
            parent = parent->parent()) {
        if (KMime::Content *sibling = nextSibling(parent)) {
            return sibling;
        }
    }

    return 0;
}

KMime::Content *MessageCore::NodeHelper::firstChild(const KMime::Content *node)
{
    if (!node) {
        return 0;
    }

    KMime::Content *child = 0;
    if (!node->contents().isEmpty()) {
        child = node->contents().at(0);
    }

    return child;
}
