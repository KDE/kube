/*
 *   Copyright 2017 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Window 2.1
import QtTest 1.0
import org.kube.framework 1.0 as Kube
import "../components/kube/contents/ui/" as Components


TestCase {
    id: logviewTestcase
    width: 400
    height: 400
    name: "LogView"

    Components.LogView {
        id: logView
    }

    function test_logview() {
        var listModel = findChild(logView, "logModel");
        verify(listModel)
        compare(listModel.count, 0)
        //ignore progress
        Kube.Fabric.postMessage(Kube.Messages.progressNotification, {})
        compare(listModel.count, 0)

        Kube.Fabric.postMessage(Kube.Messages.notification, {type: Kube.Notifications.info, message: "foobar", resource: "resource"})
        compare(listModel.count, 1)
        compare(logView.pendingError, false)

        Kube.Fabric.postMessage(Kube.Messages.notification, {"type": Kube.Notifications.error, message: "foobar", resource: "resource"})
        compare(listModel.count, 2)
        compare(logView.pendingError, true)
        compare(listModel.get(0).type, Kube.Notifications.error)
        compare(listModel.get(0).errors.count, 1)
        compare(listModel.get(0).errors.get(0).message, "foobar")
        compare(listModel.get(0).errors.get(0).resource, "resource")
    }
}
