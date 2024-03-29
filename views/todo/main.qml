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
                enabled: true,
                todos: [
                    {
                        resource: "caldavresource",
                        summary: "Todo start",
                        start: "2018-04-09T14:03:00",
                        description: "<pre>Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n<pre>",
                        doing: true,
                        subtodos: [
                            {
                                resource: "caldavresource",
                                summary: "subtodo",
                                start: "2018-04-09T14:03:00",
                                description: "<pre>Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n<pre>",
                            }
                        ]
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo due",
                        due: "2018-04-17T14:03:00",
                        description: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                        doing: true
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo due today",
                        due: "2018-04-11T14:03:00",
                        description: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                        doing: true
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo overdue",
                        due: "2018-04-03T14:03:00",
                        description: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                        doing: true
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo start and due",
                        start: "2018-04-13T14:03:00",
                        due: "2018-04-14T14:03:00",
                        description: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo"
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo done",
                        due: "2018-04-03T14:03:00",
                        description: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                        done: true
                    },
                    {
                        resource: "caldavresource",
                        summary: "Todo needs action",
                        due: "2018-04-03T14:03:00",
                        description: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                        needsAction: true
                    },
                ],
            },
            {
                id: "calendar16",
                resource: "caldavresource",
                name: "Test Calendar16",
                enabled: true,
                color: "#f67400",
                todos: [
                    {
                        resource: "caldavresource",
                        summary: "Todo from another list",
                        description: "Stuff"
                    }
                ]
            },
            {
                id: "account2calendar",
                resource: "caldavresource2",
                name: "Account2Calendar",
                color: "#f67400"
            }],
        }
        TestStore.setup(initialState)
        Kube.Context.autoUpdateDate = false
        Kube.Context.currentDate = "2018-04-11T13:04:03"
    }

    View {
        anchors.fill: parent
    }
}
