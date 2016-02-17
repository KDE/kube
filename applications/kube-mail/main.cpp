#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QStandardPaths>
#include <KPackage/PackageLoader>

#include <QDebug>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    auto package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML", "org.kde.kube.mail");
    Q_ASSERT(package.isValid());
    QQmlApplicationEngine engine(QUrl::fromLocalFile(package.filePath("mainscript")));
    return app.exec();
}
