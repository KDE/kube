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

#ifndef MESSAGECORE_NODEHELPER_H
#define MESSAGECORE_NODEHELPER_H

#include "messagecore_export.h"

namespace KMime
{
class Content;
class Message;
}

namespace MessageCore
{

/**
 * @short Contains some static functions for nagivating in KMime::Node trees.
 */
namespace NodeHelper
{

/**
   * Returns the next sibling node of the given @p node.
   * If there is no sibling node @c 0 is returned.
   */
MESSAGECORE_EXPORT KMime::Content *nextSibling(const KMime::Content *node);

/**
   * Returns the next node (child, sibling or parent) of the given @p node.
   *
   * @param node The start node for iteration.
   * @param allowChildren If @c true child nodes will be returned, otherwise only sibling or parent nodes.
   */
MESSAGECORE_EXPORT KMime::Content *next(KMime::Content *node, bool allowChildren = true);

/**
   * Returns the first child node of the given @p node.
   * If there is no child node @c 0 is returned.
   */
MESSAGECORE_EXPORT KMime::Content *firstChild(const KMime::Content *node);

}

}

#endif
