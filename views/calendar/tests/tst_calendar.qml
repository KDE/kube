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
import org.kube.test 1.0
import "../qml"

ViewTestCase {
    id: testCase
    name: "Calendar"
    width: 400
    height: 400
    when: windowShown
    visible: true

    Component {
        id: calendarViewComponent
        View {
            focus: true
            currentDate: "2018-04-11T13:04:03"
            autoUpdateDate: false
        }
    }

    function test_1start() {
        var view = createTemporaryObject(calendarViewComponent, testCase, {})
        verify(view)
    }

    // This test currently only passes when visible, because the calendarselector only works when visible (due to implementation details)
    function test_2displayEvents() {
        var initialState = {
            accounts: [
                {
                    id: "account1",
                    name: "Test Account"
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
                }
            ],
            calendars: [{
                id: "calendar1",
                resource: "caldavresource",
                name: "Test Calendar",
                color: "#af1a6a",
                enabled: true,
                events: [
                    {
                        resource: "caldavresource",
                        summary: "Event1",
                        starts: "2018-04-09T14:03:00",
                        organizer: "organizer@example.org",
                        attendees: [{email: "attendee1@example.org"}, {email: "identity@example.org"}]
                    },
                    // Day-long events
                    {
                        resource: "caldavresource",
                        summary: "Test day-long event1",
                        description: "This is test day-long event #1",
                        starts: "2018-04-10T00:00:00",
                        ends:   "2018-04-14T00:00:00",
                        allDay: true
                    },
                ],
            },
            {
                id: "calendar16",
                resource: "caldavresource",
                name: "Test Calendar16",
                color: "#f67400",
                enabled: false
            }],
        }
        TestStore.setup(initialState)
        var view = createTemporaryObject(calendarViewComponent, testCase, {})

        var calendarSelector = findChild(view, "calendarSelector");
        verify(calendarSelector)
        //Only testable from qt 5.14 on
        // tryCompare(calendarSelector, "enabledEntities", ["calendar1"])

        var weekView = findChild(view, "weekView");
        verify(weekView)

        var eventOccurrenceModel = findChild(weekView, "eventOccurrenceModel");
        verify(eventOccurrenceModel)
        tryVerify(function(){ return eventOccurrenceModel.rowCount() == 1; })
    }

}
