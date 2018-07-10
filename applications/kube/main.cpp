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
#include <cstdlib>
#include <ostream>
#include <sstream>
#include <thread>
#include <chrono>
#ifndef Q_OS_WIN
#include <execinfo.h>
#include <unistd.h>
#include <cxxabi.h>
#include <dlfcn.h>
#else
#include <io.h>
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
#include <QIcon>
#include <QStandardPaths>
#include <QResource>
#include <sink/store.h>

#include "framework/src/keyring.h"
#include "kube_version.h"

//Print a demangled stacktrace
void printStacktrace()
{
#ifndef Q_OS_WIN
    int skip = 1;
    void *callstack[128];
    const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
    char buf[1024];
    int nFrames = backtrace(callstack, nMaxFrames);
    char **symbols = backtrace_symbols(callstack, nFrames);

    std::ostringstream trace_buf;
    for (int i = skip; i < nFrames; i++) {
        // printf("%s\n", symbols[i]);
        Dl_info info;
        if (dladdr(callstack[i], &info) && info.dli_sname) {
            char *demangled = NULL;
            int status = -1;
            if (info.dli_sname[0] == '_') {
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            }
            snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
                    i, int(2 + sizeof(void*) * 2), callstack[i],
                    status == 0 ? demangled :
                    info.dli_sname == 0 ? symbols[i] : info.dli_sname,
                    (char *)callstack[i] - (char *)info.dli_saddr);
            free(demangled);
        } else {
            snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
                    i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
        }
        trace_buf << buf;
    }
    free(symbols);
    if (nFrames == nMaxFrames) {
        trace_buf << "[truncated]\n";
    }
    std::cerr << trace_buf.str();
#endif
}

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
    app.setFont(QFont{"Noto Sans", app.font().pointSize(), QFont::Normal});


    QCommandLineParser parser;
    parser.setApplicationDescription("A communication and collaboration client.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"k", "keyring"},
        QCoreApplication::translate("main", "To automatically unlock the keyring pass in a keyring in the form of {\"accountId\": {\"resourceId\": \"secret\", *}}"), "keyring dictionary"}
    );
    parser.addOption({{"u", "upgrade"}, "", ""});
    parser.process(app);

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

