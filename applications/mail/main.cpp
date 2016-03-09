#include <QApplication>
#include <QQmlApplicationEngine>

#include <QStandardPaths>
#include <KPackage/PackageLoader>
#include <QQuickImageProvider>
#include <QIcon>

#include <QDebug>

class KubeImageProvider : public QQuickImageProvider
{
public:
    KubeImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) Q_DECL_OVERRIDE
    {
        const auto icon = QIcon::fromTheme(id);
        auto expectedSize = requestedSize;
        if (!icon.availableSizes().contains(requestedSize) && !icon.availableSizes().isEmpty()) {
            expectedSize = icon.availableSizes().first();
        }
        const auto pixmap = icon.pixmap(expectedSize);
        if (size) {
            *size = pixmap.size();
        }
        return pixmap;
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    auto package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML", "org.kube.components.mail");
    Q_ASSERT(package.isValid());
    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("kube"), new KubeImageProvider);
    engine.load(QUrl::fromLocalFile(package.filePath("mainscript")));
    return app.exec();
}
