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
import QtTest 1.0
import org.kube.framework 1.0 as Kube
import org.kube.test 1.0
import "../qml"

TestCase {
    id: logviewTestcase
    width: 400
    height: 400
    name: "LogView"

    View {
        id: logView
    }

    function test_logview() {
        var listModel = findChild(logView, "logModel");
        verify(listModel)
        compare(listModel.rowCount(), 0)
        //ignore progress
        Kube.Fabric.postMessage(Kube.Messages.progressNotification, {})
        compare(listModel.rowCount(), 0)

        Kube.Fabric.postMessage(Kube.Messages.notification, {type: Kube.Notifications.info, message: "foobar", resource: "resource"})
        compare(listModel.rowCount(), 1)
        compare(logView.pendingError, false)

        Kube.Fabric.postMessage(Kube.Messages.notification, {"type": Kube.Notifications.error, message: "foobar", resource: "resource"})
        compare(listModel.rowCount(), 2)
        compare(logView.pendingError, true)

        //FIXME test the model contents again
        //Yes, this is ridiculous
        // compare(listModel.data(listModel.index(0, 0), Kube.LogModel.Type), Kube.Notifications.error)
        // compare(listModel.get(0).errors.rowCount(), 1)
        // compare(listModel.get(0).errors.get(0).message, "foobar")
        // compare(listModel.get(0).errors.get(0).resource, "resource")

        Kube.Fabric.postMessage(Kube.Messages.notification, {"type": Kube.Notifications.error, "subtype": "merge", message: "merge1", resource: "resource1"})
        compare(listModel.rowCount(), 3)
        Kube.Fabric.postMessage(Kube.Messages.notification, {"type": Kube.Notifications.error, "subtype": "merge", message: "merge2", resource: "resource2"})
        compare(listModel.rowCount(), 3)

        // compare(listModel.get(0).errors.rowCount(), 2)
        // compare(listModel.get(0).errors.get(0).message, "merge2")
        // compare(listModel.get(0).errors.get(0).resource, "resource2")
    }
}
