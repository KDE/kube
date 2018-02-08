/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include <QtQml>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <sink/test.h>

#include "teststore.h"

class TestPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void initializeEngine(QQmlEngine *engine, const char *uri) Q_DECL_OVERRIDE
    {
        Q_UNUSED(engine);
        Q_UNUSED(uri);
        Sink::Test::initTest();
    }

    void registerTypes (const char *uri) Q_DECL_OVERRIDE
    {
        qmlRegisterSingletonType<Kube::TestStore>(uri, 1, 0, "TestStore", [] (QQmlEngine *, QJSEngine *) -> QObject* {
            return new Kube::TestStore;
        });
    }
};



#include "testplugin.moc"
