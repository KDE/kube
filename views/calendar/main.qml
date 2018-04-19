/*
 * Copyright (C) 2018 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Window 2.0

import org.kube.framework 1.0 as Kube
import org.kube.test 1.0
import "qml"

ApplicationWindow {
    id: app
    height: Screen.desktopAvailableHeight * 0.8
    width: Screen.desktopAvailableWidth * 0.8

    Component.onCompleted: {
        var initialState = {
            accounts: [{
                id: "account1",
                name: "Test Account"
            }],
            identities: [{
                account: "account1",
                name: "Test Identity",
                address: "identity@example.org"
            }],
            resources: [{
                id: "caldavresource",
                account: "account1",
                type: "caldav",
            }],
            calendars: [{
                id: "calendar1",
                resource: "caldavresource",
                name: "Test Calendar",
                events: [
                    {
                        resource: "caldavresource",
                        summary: "Test Event1",
                        description: "This is test event #1",
                        starts: "2018-04-10T14:03:00",
                        ends:   "2018-04-10T17:03:00",
                    },
                    {
                        resource: "caldavresource",
                        summary: "Test Event2",
                        description: "This is test event #2",
                        starts: "2018-04-11T09:03:00",
                        ends:   "2018-04-11T14:03:00",
                    },
                    {
                        resource: "caldavresource",
                        summary: "Test Event3",
                        description: "This is test event #3",
                        starts: "2018-04-11T10:03:00",
                        ends:   "2018-04-11T15:00:00",
                    },
                ],
            }],
        }
        TestStore.setup(initialState)
    }

    View {
        anchors.fill: parent
    }
}
