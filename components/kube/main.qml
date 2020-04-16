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

import org.kube.components.kube 1.0 as KubeComponent
import org.kube.framework 1.0 as Kube
import org.kube.test 1.0
import "qml"


KubeComponent.Kube {
    id: app
    height: Screen.desktopAvailableHeight * 0.8
    width: Screen.desktopAvailableWidth * 0.8

    function setHour(date, hour) {
        var d = date
        return new Date(d.setHours(hour))
    }

    function addDays(date, days) {
        return new Date(date.getTime() + (24*60*60*1000 * days))
    }

    function getFirstDayOfWeek(date) {
        //This works with negative days to get to the previous month
        //Date.getDate() is the day of the month, Date.getDay() is the day of the week
        return new Date(date.getFullYear(), date.getMonth(), date.getDate() - (date.getDay() - Qt.locale().firstDayOfWeek))
    }

    function dayOfWeek(date, day) {
        //This works with negative days to get to the previous month
        //Date.getDate() is the day of the month, Date.getDay() is the day of the week
        return new Date(date.getFullYear(), date.getMonth(), date.getDate() - (date.getDay() - day))
    }

    Component.onCompleted: {
        var now = new Date()
        var firstDayOfWeek = getFirstDayOfWeek(now)
        var initialState = {
            accounts: [
                {
                    id: "account1",
                    name: "Work"
                },
                {
                    id: "account2",
                    name: "Private"
                },
            ],
            identities: [{
                account: "account1",
                name: "John Doe",
                address: "john@example.org"
            }],
            resources: [
                {
                    id: "resource1",
                    account: "account1",
                    type: "dummy",
                },
                {
                    id: "resource2",
                    account: "account2",
                    type: "dummy",
                }
            ],
            folders: [{
                    id: "folder1",
                    resource: "resource1",
                    name: "Inbox",
                    specialpurpose: ["inbox"],
                    mails: [
                        {
                            messageId: "<msg2@test.com>",
                            date: setHour(now, 17),
                            to: ["\"John Doe\"<john@example.org>", "\"Andrea Mueller\"<andrea@example.org>"],
                            from: "\"Jane Doe\"<jane@example.org>",
                            subject: "Kube 0.8 is out!",
                            body: "Hey,\n\nDid you see? Kube 0.8 is finally out!\n\nGet it now on www.kube-project.com\n\nCheers, Jane",
                            unread: false,
                            important: true
                        },
                        {
                            inReplyTo: "<msg2@test.com>",
                            date: setHour(now, 18),
                            from: "\"Andrea Mueller\"<andrea@example.org>",
                            subject: "RE: Kube 0.8 is out!",
                            body: "Looking good indeed =)",
                            to: ["\"Jane Doe\"<jane@example.org>", "\"John Doe\"<john@example.org>"],
                            important: true,
                            unread: false
                        },
                        {
                            messageId: "<msg1@test.com>",
                            date: setHour(now, 15),
                            subject: "subject1",
                            body: "body",
                            to: ["to@example.org"],
                            from: "\"Jane Doe\"<jane@example.org>",
                            unread: true
                        },
                        {
                            inReplyTo: "<msg1@test.com>",
                            date: setHour(now, 16),
                            subject: "RE: Meeting report",
                            body: "body2",
                            to: ["to@example.org"],
                            from: "\"Jane Doe\"<jane@example.org>",
                            unread: true
                        },
                        {
                            date: setHour(now, 14),
                            subject: " ❆ ❆ ❆ Winter is coming ❆ ❆ ❆",
                            body: "UTF-8 Madness Umlauts:öüä Snowflake:❆ Heart:♥",
                            to: ["öüä@example.org"],
                            from: "\"Jane Doe\"<jane@example.org>",
                            unread: true
                        },
                        {
                            date: addDays(now, -1),
                            subject: "Great Post!",
                            to: ["öüä@example.org"],
                            from: "\"Jane Doe\"<jane@example.org>",
                            unread: true
                        },
                        {
                            date: addDays(now, -3),
                            subject: "Last weeks meeting",
                            to: ["öüä@example.org"],
                            from: "\"Evan Smith\"<jane@example.org>",
                            unread: false
                        },
                        {
                            date: addDays(now, -4),
                            subject: "Tennis tomorrow?",
                            to: ["öüä@example.org"],
                            from: "\"Anna Cole\"<anna@example.org>",
                            unread: false
                        },
                        {
                            date: addDays(now, -4),
                            subject: "Project report",
                            to: ["öüä@example.org"],
                            from: "\"Phil Barry\"<phil@example.org>",
                            unread: false
                        },
                        {
                            date: addDays(now, -5),
                            subject: "Dummy",
                            from: "test@example.org",
                        },
                        {
                            date: addDays(now, -5),
                            subject: "Dummy",
                            from: "test@example.org",
                        },
                        {
                            date: addDays(now, -5),
                            subject: "Dummy",
                            from: "test@example.org",
                        },
                        {
                            date: addDays(now, -5),
                            subject: "Dummy",
                            from: "test@example.org",
                        },
                        {
                            date: addDays(now, -6),
                            subject: "Before last week",
                            from: "test@example.org",
                        },
                        {
                            date: addDays(now, -7),
                            subject: "Last week",
                            from: "test@example.org",
                        },
                    ]
                },
                {
                    id: "folder2",
                    resource: "resource1",
                    specialpurpose: ["drafts"],
                    name: "Drafts",
                    mails: [
                        {
                            date: setHour(now, 17),
                            to: ["\"Jane Doe\"<jane@example.org>", "\"Andrea Mueller\"<andrea@example.org>"],
                            from: "\"John Doe\"<john@example.org>",
                            subject: "Kube 0.8 is out!",
                            body: "Hey,\n\nDid you see? Kube 0.8 is finally out!\n\nGet it now on www.kube-project.com\n\nCheers, Jane",
                            unread: false,
                            important: true
                        },
                    ]
                },
                {
                    id: "folder3",
                    resource: "resource1",
                    specialpurpose: ["sent"],
                    name: "Sent"
                },
                {
                    id: "folder4",
                    resource: "resource1",
                    specialpurpose: ["trash"],
                    name: "Trash"
                },
                {
                    id: "folder5",
                    resource: "resource1",
                    name: "Spam"
                }
            ],
            addressbooks: [{
                id: "addressbook1",
                resource: "resource1",
                name: "Personal Addressbook",
                contacts: [
                    {
                        uid: "uid1",
                        givenname: "John",
                        familyname: "Doe",
                        email: ["doe@example.org"],
                    },
                    {
                        uid: "uid2",
                        givenname: "Andrea",
                        familyname: "Mueller",
                        email: ["andrea@example.org"],
                    },
                    {
                        uid: "uid3",
                        givenname: "Anna",
                        familyname: "Cole",
                        email: ["anna@example.org"],
                    },
                    {
                        uid: "uid4",
                        givenname: "Jane",
                        familyname: "Doe",
                        email: ["jane@example.org"],
                    },
                    {
                        uid: "uid5",
                        givenname: "Phil",
                        familyname: "Barry",
                        email: ["phil@example.org"],
                    }

                ],
            }],
            calendars: [{
                id: "calendar1",
                resource: "resource1",
                name: "Calendar",
                color: "#af1a6a",
                contentTypes: ["event"],
                enabled: true,
                events: [
                    {
                        summary: "Monday morning standup",
                        starts: setHour(firstDayOfWeek, 9),
                    },
                    {
                        summary: "Lunch with Claire",
                        starts: setHour(firstDayOfWeek, 12),
                        ends: setHour(firstDayOfWeek, 13),
                    },
                    {
                        summary: "Meeting with Nik",
                        starts: setHour(firstDayOfWeek, 14),
                        ends: setHour(firstDayOfWeek, 16),
                    },
                    {
                        summary: "Tennis?",
                        description: "This is test event #2",
                        starts: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 1), 13),
                        ends: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 1), 15),
                    },
                    {
                        summary: "Dinner with Julia",
                        description: "This is test event #3",
                        starts: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 1), 18),
                        ends: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 1), 20),
                    },
                    {
                        summary: "Mischa in Zurich",
                        description: "This is test event #4",
                        starts: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 2), 8),
                        ends: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 4), 16),
                    },
                    {
                        summary: "Draft Report",
                        description: "This is test event #3",
                        starts: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 4), 13),
                        ends: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 4), 17),
                    },
                    {
                        summary: "Send Report",
                        description: "This is test event #3",
                        starts: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 5), 12),
                    },
                ],
            },
            {
                id: "calendar2",
                resource: "resource1",
                name: "Vacations",
                color: "#00cc4b",
                contentTypes: ["event"],
                enabled: true,
                events: [
                    {
                        summary: "Sandra is off",
                        description: "This is test day-long event #1",
                        starts: dayOfWeek(now, Qt.locale().firstDayOfWeek),
                        ends: dayOfWeek(now, Qt.locale().firstDayOfWeek + 7),
                        allDay: true,
                    },
                    {
                        summary: "Jeremy is off",
                        description: "This is test day-long event #2",
                        starts: dayOfWeek(now, Qt.locale().firstDayOfWeek + 4),
                        allDay: true,
                    },
                    {
                        summary: "John's early weekend",
                        description: "This is test event #3",
                        starts: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 5), 14),
                        ends: setHour(dayOfWeek(now, Qt.locale().firstDayOfWeek + 5), 24),
                    },
                ]
            },
            {
                id: "calendar3",
                resource: "resource1",
                name: "Tasks",
                color: "#af1a6a",
                contentTypes: ["todo"],
                enabled: true,
                todos: [
                    {
                        summary: "Prepare meeting",
                        starts: "2018-04-09T14:03:00",
                        doing: true
                    },
                    {
                        summary: "Release Kube",
                        due: "2018-04-09T14:03:00",
                        doing: true
                    },
                    {
                        summary: "Blog about latest improvements",
                        doing: true
                    },
                    {
                        summary: "Pay bills",
                        doing: true
                    },
                    {
                        summary: "Get a gift for Pam",
                        doing: true
                    },
                ],
            },
            {
                id: "account2calendar",
                resource: "resource2",
                name: "Account2Calendar",
                color: "#f67400",
                enabled: true
            }],
        }
        TestStore.setup(initialState)
    }
}
