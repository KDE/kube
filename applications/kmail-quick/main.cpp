#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine(QUrl::fromLocalFile("/home/chrigi/kdebuild/akonadinext/source/kontact-quick/applications/kmail-quick/package/contents/ui/main.qml"));
    return app.exec();
}
