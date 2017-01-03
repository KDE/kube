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
    WebUrlRequestInterceptor(QObject *p = Q_NULLPTR) : QWebEngineUrlRequestInterceptor{}
    {}

    void interceptRequest(QWebEngineUrlRequestInfo &info)
    {
        qWarning() << info.requestMethod() << info.requestUrl();
        QDesktopServices::openUrl(info.requestUrl());
        info.block(true);
        //TODO handle mailto to open a composer
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QtWebEngine::initialize();
    WebUrlRequestInterceptor *wuri = new WebUrlRequestInterceptor();
    QQuickWebEngineProfile::defaultProfile()->setRequestInterceptor(wuri);

    auto package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML", "org.kube.components.mail");
    Q_ASSERT(package.isValid());
    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("kube"), new KubeImageProvider);
    engine.load(QUrl::fromLocalFile(package.filePath("mainscript")));
    return app.exec();
}

#include "main.moc"
