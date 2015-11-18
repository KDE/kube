#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QStandardPaths>

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KPackage/PackageStructure>

#include <QDebug>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    auto mainFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kpackage/genericqml/org.kde.pim.kmail-quick/contents/ui/main.qml", QStandardPaths::LocateFile);
    QQmlApplicationEngine engine(QUrl::fromLocalFile(mainFile));
    return app.exec();
}
