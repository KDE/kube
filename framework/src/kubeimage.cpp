#include "kubeimage.h"

#include <QQuickWindow>
#include <QSGTexture>
#include <QSGSimpleTextureNode>

KubeImage::KubeImage(QQuickItem *parent)
    :QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents);
}

QSGNode *KubeImage::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData)
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
