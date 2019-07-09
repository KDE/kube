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
                    name: "Test Account2 With A Very Long Name"
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
                events: [
                    {
                        resource: "caldavresource",
                        summary: "Short event1!",
                        starts: "2018-04-09T14:03:00",
                        organizer: "organizer@example.org",
                        attendees: [{email: "attendee1@example.org"}, {email: "identity@example.org"}]
                    },
                    {
                        resource: "caldavresource",
                        summary: "Short event2!",
                        starts: "2018-04-09T14:03:00",
                        organizer: "organizer@example.org",
                        attendees: [{email: "attendee1@example.org"}, {email: "identity@example.org"}]
                    },
                    {
                        resource: "caldavresource",
                        summary: "Short event3!",
                        starts: "2018-04-09T14:03:00",
                    },
                    {
                        resource: "caldavresource",
                        summary: "Test Event1 with a waaaaaaaay to long summary. Why don't you just use the description you fool!",
                        description: "This is test event #1",
                        starts: "2018-04-10T14:03:00",
                        ends:   "2018-04-10T16:03:00",
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
                        starts: "2018-04-11T10:00:00",
                        ends:   "2018-04-11T15:00:00",
                    },
                    {
                        resource: "caldavresource",
                        summary: "Test Event4",
                        description: "This is test event #4",
                        starts: "2018-04-12T03:00:00",
                        ends:   "2018-04-14T22:00:00",
                    },
                    {
                        resource: "caldavresource",
                        summary: "!!! Test Event5",
                        description: "!!! This is test event #5",
                        starts: "2018-04-22T22:00:00",
                        ends:   "2018-04-25T03:00:00",
                    },

                    // Day-long events
                    {
                        resource: "caldavresource",
                        summary: "Test day-long event1",
                        description: "This is test day-long event #1",
                        starts: "2018-04-10T00:00:00",
                        ends:   "2018-04-14T00:00:00",
                        allDay: true,
                    },
                    {
                        resource: "caldavresource",
                        summary: "Test day-long event2",
                        description: "This is test day-long event #2",
                        starts: "2018-04-11",
                        allDay: true,
                    },
                    {
                        resource: "caldavresource",
                        summary: "Test day-long event3",
                        description: "This is test day-long event #3",
                        starts: "2018-04-01T00:00:00",
                        ends:   "2018-04-13T00:00:00",
                        allDay: true,
                    },
                    {
                        resource: "caldavresource",
                        summary: "Test day-long event4",
                        description: "This is test day-long event #4",
                        starts: "2018-04-01T00:00:00",
                        ends:   "2018-04-25T00:00:00",
                        allDay: true,
                    },
                    {
                        resource: "caldavresource",
                        summary: "!!! Test day-long event5",
                        description: "!!! This is test day-long event #5",
                        starts: "2018-04-01T00:00:00",
                        ends:   "2018-04-05T00:00:00",
                        allDay: true,
                    },
                    {
                        resource: "caldavresource",
                        summary: "!!! Test day-long event6",
                        description: "!!! This is test day-long event #6",
                        starts: "2018-04-23T00:00:00",
                        ends:   "2018-04-25T00:00:00",
                        allDay: true,
                    },
                    {
                        resource: "caldavresource",
                        summary: "Test day-long event7",
                        description: "This is test day-long event #7",
                        starts: "2018-04-12",
                        allDay: true,
                    },
                ],
            },
            {
                id: "calendar2",
                resource: "caldavresource",
                name: "Test Calendar2",
                color: "#00cc4b"
            },
            {
                id: "calendar3",
                resource: "caldavresource",
                name: "Test Calendar3 Loooooooooooooooooooooooooooooong Name",
                color: "#00cc4b"
            },
            {
                id: "calendar4",
                resource: "caldavresource",
                name: "Test Calendar4",
                color: "#cc0000"
            },
            {
                id: "calendar5",
                resource: "caldavresource",
                name: "Test Calendar5",
                color: "#cc0000"
            },
            {
                id: "calendar6",
                resource: "caldavresource",
                name: "Test Calendar6",
                color: "#f67400"
            },
            {
                id: "calendar7",
                resource: "caldavresource",
                name: "Test Calendar7",
                color: "#f67400"
            },
            {
                id: "calendar8",
                resource: "caldavresource",
                name: "Test Calendar8",
                color: "#f67400"
            },
            {
                id: "calendar9",
                resource: "caldavresource",
                name: "Test Calendar9",
                color: "#f67400"
            },
            {
                id: "calendar10",
                resource: "caldavresource",
                name: "Test Calendar 10",
                color: "#f67400"
            },
            {
                id: "calendar11",
                resource: "caldavresource",
                name: "Test Calendar11",
                color: "#f67400"
            },
            {
                id: "calendar12",
                resource: "caldavresource",
                name: "Test Calendar12",
                color: "#f67400"
            },
            {
                id: "calendar13",
                resource: "caldavresource",
                name: "Test Calendar13",
                color: "#f67400"
            },
            {
                id: "calendar14",
                resource: "caldavresource",
                name: "Test Calendar14",
                color: "#f67400"
            },
            {
                id: "calendar15",
                resource: "caldavresource",
                name: "Test Calendar15",
                color: "#f67400"
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
        currentDate: "2018-04-11T13:04:03"
        autoUpdateDate: false
    }
}
