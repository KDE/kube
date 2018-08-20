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

#include <QtGlobal>
#include <signal.h>
#include <csignal>
#include <iostream>
#include <thread>
#include <chrono>
#ifndef Q_OS_WIN
#include <unistd.h>
#else
#include <process.h>
#endif

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QFont>
#include <QDebug>
#include <QTimer>
#include <QQmlContext>
#include <QStandardPaths>
#include <QLockFile>
#include <QDir>
#include <sink/store.h>

#include "backtrace.h"
#include "framework/src/keyring.h"
#include "kube_version.h"

static int sCounter = 0;

void crashHandler(int signal)
{
    //Guard against crashing in here
    if (sCounter > 1) {
        std::_Exit(EXIT_FAILURE);
    }
    sCounter++;

    if (signal == SIGABRT) {
        std::cerr << "SIGABRT received\n";
    } else if (signal == SIGSEGV) {
        std::cerr << "SIGSEV received\n";
    } else {
        std::cerr << "Unexpected signal " << signal << " received\n";
    }

    printStacktrace();

    std::fprintf(stdout, "Sleeping for 10s to attach a debugger: gdb attach %i\n", getpid());
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // std::system("exec gdb -p \"$PPID\" -ex \"thread apply all bt\"");
    // This only works if we actually have xterm and X11 available
    // std::system("exec xterm -e gdb -p \"$PPID\"");

    std::_Exit(EXIT_FAILURE);
}

void terminateHandler()
{
    // std::exception_ptr exptr = std::current_exception();
    // if (exptr != 0)
    // {
    //     // the only useful feature of std::exception_ptr is that it can be rethrown...
    //     try {
    //         std::rethrow_exception(exptr);
    //     } catch (std::exception &ex) {
    //         std::fprintf(stderr, "Terminated due to exception: %s\n", ex.what());
    //     } catch (...) {
    //         std::fprintf(stderr, "Terminated due to unknown exception\n");
    //     }
    // } else {
        std::fprintf(stderr, "Terminated due to unknown reason.\n");
    // }
    std::abort();
}


QString findFile(const QString file, const QStringList importPathList)
{
    for (const auto &path : importPathList) {
        const QString f = path + file;
        if (QFileInfo::exists(f)) {
            return f;
        }
    }
    return {};
}

int main(int argc, char *argv[])
{
    std::signal(SIGSEGV, crashHandler);
    std::signal(SIGABRT, crashHandler);
    std::set_terminate(terminateHandler);

    //Instead of QtWebEngine::initialize();
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QApplication app(argc, argv);
    app.setApplicationName("kube");
    app.setApplicationVersion(QString("%1 Branch: %2 Commit: %3").arg(kube_VERSION_STRING).arg(kube_BRANCH).arg(kube_COMMIT));
    auto fontSize = app.font().pointSize();
#ifdef Q_OS_UNIX
    if (qgetenv("QT_QPA_PLATFORMTHEME").isEmpty()) {
        //The hardcoded default in qgenericunixthemes.cpp of 9 is tiny, so we default to something larger.
        fontSize = 11;
    }
#endif
    qWarning() << "Font size:" << fontSize;
    app.setFont(QFont{"Noto Sans", fontSize, QFont::Normal});

    QCommandLineParser parser;
    parser.setApplicationDescription("A communication and collaboration client.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"k", "keyring"},
        QCoreApplication::translate("main", "To automatically unlock the keyring pass in a keyring in the form of {\"accountId\": {\"resourceId\": \"secret\", *}}"), "keyring dictionary"}
    );
    parser.addOption({{"l", "lockfile"}, "Use a lockfile to enforce that only a single instance can be started.", ""});
    parser.process(app);


    static QString location = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kube";
    QDir{}.mkpath(location);
    QLockFile lockfile(location + QString("/%1.lock").arg(QString("kube")));
    lockfile.setStaleLockTime(500);
    if (parser.isSet("lockfile")) {
        if (!lockfile.tryLock(0)) {
            const auto error = lockfile.error();
            if (error == QLockFile::LockFailedError) {
                qint64 pid;
                QString hostname, appname;
                lockfile.getLockInfo(&pid, &hostname, &appname);
                qWarning() << "Failed to acquire exclusive lock. You can only run one instance of Kube.";
                qWarning() << "Pid:" << pid << "Host:" << hostname << "App:" << appname;
            } else {
                qWarning() << "Error while trying to acquire exclusive lock: " << error;
            }
            return -1;
        }
    }

    if (parser.isSet("keyring")) {
        auto keyringDict = parser.value("keyring");
        auto json = QJsonDocument::fromJson(keyringDict.toUtf8());
        if (!json.isObject()) {
            qWarning() << "Not a valid keyring dict " << keyringDict;
            return -1;
        }
        auto object = json.object();
        for (const auto accountId : object.keys()) {
            auto dict = object.value(accountId).toObject();
            for (const auto resourceId : dict.keys()) {
                Kube::AccountKeyring{accountId.toUtf8()}.storePassword(resourceId.toUtf8(), dict.value(resourceId).toString().toUtf8());
            }
        }
    }

    {
        QQmlContext *rootContext = nullptr;
        bool upgradeComplete = false;
        Sink::Store::upgrade().then([&] (Sink::Store::UpgradeResult upgradeExecuted) {
            upgradeComplete = !upgradeExecuted.upgradeExecuted;
            if (rootContext) {
                rootContext->setContextProperty("upgradeComplete", true);
            }
        }).exec();
        if (!upgradeComplete) {
            QQmlApplicationEngine engine;
            engine.rootContext()->setContextProperty("upgradeComplete", false);
            rootContext = engine.rootContext();
            engine.load(QUrl::fromLocalFile(findFile("/org/kube/components/kube/upgrade.qml", engine.importPathList())));
            return app.exec();
        }
    }

    QQmlApplicationEngine engine;
    //For windows
    engine.addImportPath(QCoreApplication::applicationDirPath() + QStringLiteral("/../qml"));
    const auto file = "/org/kube/components/kube/main.qml";
    const auto mainFile = findFile(file, engine.importPathList());
    if (mainFile.isEmpty()) {
        qWarning() << "Failed to find " << file;
        qWarning() << "Searched: " << engine.importPathList();
        return -1;
    }
    engine.load(QUrl::fromLocalFile(mainFile));
    return app.exec();
}

