#include <QApplication>
#include <QQmlApplicationEngine>

#include <QStandardPaths>
#include <KPackage/PackageLoader>
#include <QQuickImageProvider>
#include <QIcon>
#include <QtWebEngine>
#include <QDesktopServices>

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
        //The platform theme plugin can overwrite our setting again once it gets loaded,
        //so we check on every icon load request...
        if (QIcon::themeName() != "kube") {
            QIcon::setThemeName("kube");
        }
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

class WebUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    WebUrlRequestInterceptor(QObject *p = Q_NULLPTR) : QWebEngineUrlRequestInterceptor{p}
    {}

    void interceptRequest(QWebEngineUrlRequestInfo &info)
    {
        qDebug() << info.requestMethod() << info.requestUrl() << info.resourceType() << info.navigationType();
        const bool isNavigationRequest = info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeMainFrame;
        if (isNavigationRequest) {
            QDesktopServices::openUrl(info.requestUrl());
            info.block(true);
        }
        //TODO handle mailto to open a composer
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QtWebEngine::initialize();
    WebUrlRequestInterceptor *wuri = new WebUrlRequestInterceptor();
    QQuickWebEngineProfile::defaultProfile()->setRequestInterceptor(wuri);
    QIcon::setThemeName("kube");

    auto package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML", "org.kube.components.kube");
    Q_ASSERT(package.isValid());
    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("kube"), new KubeImageProvider);
    engine.load(QUrl::fromLocalFile(package.filePath("mainscript")));
    return app.exec();
}

#include "main.moc"
