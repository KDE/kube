/*
    Copyright (c) 2021 Christian Mollekopf <christian@mkpf.ch>

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

#include <QTimer>
#include <functional>

/**
 * A signal throttler/debouncer
 *
 * The callback is initially slightly delayed, to debounce multiple calls in quick succession.
 * The delay used for this is small to provide responsiveness. Successive calls are then batched into a single callback after  mInterval ms.
 */
class Debouncer {
    public:
        Debouncer(int interval, std::function<void()> callback, QObject *guard)
            :mInterval{interval},
            mCallback{callback}
        {
            mRefreshTimer.setSingleShot(true);
            QObject::connect(&mRefreshTimer, &QTimer::timeout, guard, [this] {
                //Avoid calling after the timeout if there was no trigger inbetween
                if (!mCalledAlready) {
                    mCallback();
                }
            });

            mDelayTimer.setSingleShot(true);
            QObject::connect(&mDelayTimer, &QTimer::timeout, guard, [this] {
                mCalledAlready = true;
                mCallback();
            });
        }

        void trigger() {
            mCalledAlready = false;
            if (!mRefreshTimer.isActive()) {
                mRefreshTimer.start(mInterval);
                mDelayTimer.start(30);
            }
        }

    private:
        int mInterval;
        bool mCalledAlready{false};
        QTimer mRefreshTimer;
        QTimer mDelayTimer;
        std::function<void()> mCallback;
};
