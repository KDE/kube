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

#include <QApplication>
#include <QQmlApplicationEngine>

#include <QStandardPaths>
#include <KPackage/PackageLoader>
#include <QQuickImageProvider>
#include <QIcon>
#include <QtWebEngine>

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
        //Get the largest size that is still smaller or equal than requested
        //Except if we only have larger sizes, then just pick the closest one
        bool first = true;
        for (const auto s : icon.availableSizes()) {
            if (first && s.width() > requestedSize.width()) {
                expectedSize = s;
                break;
            }
            first = false;
            if (s.width() <= requestedSize.width()) {
                expectedSize = s;
            }
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
    app.setFont(QFont{"Noto Sans", app.font().pointSize(), QFont::Normal});

    QtWebEngine::initialize();
    QIcon::setThemeName("kube");

    auto package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML", "org.kube.components.kube");
    Q_ASSERT(package.isValid());
    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("kube"), new KubeImageProvider);
    engine.load(QUrl::fromLocalFile(package.filePath("mainscript")));
    return app.exec();
}

