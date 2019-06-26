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
            accounts: [
                {
                    id: "account1",
                    name: "Test Account"
                },
                {
                    id: "account2",
                    name: "Test Account2"
                },
            ],
            identities: [{
                account: "account1",
                name: "Test Identity",
                address: "identity@example.org"
            }],
            resources: [
                {
                    id: "caldavresource",
                    account: "account1",
                    type: "caldav",
                },
                {
                    id: "caldavresource2",
                    account: "account2",
                    type: "caldav",
                }
            ],
            calendars: [{
                id: "calendar1",
                resource: "caldavresource",
                name: "Test Calendar",
                color: "#af1a6a",
                todos: [
                    {
                        resource: "caldavresource",
                        summary: "Todo start",
                        starts: "2018-04-09T14:03:00",
                        description: "<pre>Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n<pre>",
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo due",
                        due: "2018-04-09T14:03:00",
                        description: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo"
                    },
                ],
            },
            {
                id: "calendar16",
                resource: "caldavresource",
                name: "Test Calendar16",
                color: "#f67400"
            },
            {
                id: "account2calendar",
                resource: "caldavresource2",
                name: "Account2Calendar",
                color: "#f67400"
            }],
        }
        TestStore.setup(initialState)
    }

    View {
        anchors.fill: parent
    }
}
