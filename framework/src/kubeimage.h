
#include <QQuickItem>

class KubeImage : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QByteArray imageData MEMBER mImageData)
public:
    KubeImage(QQuickItem *parent = nullptr);

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData) Q_DECL_OVERRIDE;

private:
    QByteArray mImageData;
};
