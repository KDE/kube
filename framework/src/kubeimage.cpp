/*
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
#include "kubeimage.h"

#include <QQuickWindow>
#include <QSGTexture>
#include <QSGSimpleTextureNode>

KubeImage::KubeImage(QQuickItem *parent)
    :QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents);
}

QSGNode *KubeImage::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!node) {
        node = new QSGSimpleTextureNode();
        auto img = QImage::fromData(mImageData);
        QSGTexture *texture = window()->createTextureFromImage(img);
        node->setTexture(texture);
    }
    node->setRect(boundingRect());
    return node;
}
