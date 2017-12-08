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
#pragma once

#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QPointer>

template<typename T>
void asyncRun(QObject *object, std::function<T()> run, std::function<void(T)> continuation)
{
    auto guard = QPointer<QObject>{object};
    auto future = QtConcurrent::run(run);
    auto watcher = new QFutureWatcher<T>;
    QObject::connect(watcher, &QFutureWatcher<T>::finished, watcher, [watcher, continuation, guard]() {
        if (guard) {
            continuation(watcher->future().result());
        }
        delete watcher;
    });
    watcher->setFuture(future);
}

