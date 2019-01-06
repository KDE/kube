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
import org.kube.framework 1.0 as Kube
import "../qml"

ViewTestCase {
    id: testCase
    name: "EventEditor"

    Component {
        id: editorComponent
        EventEditor {
            focus: true
        }
    }
    Component {
        id: controllerComponent
        Kube.EventController {
        }
    }

    function test_1start() {
        var editor = createTemporaryObject(editorComponent, testCase, {})
        verify(editor)
    }

    function test_2loadStartDate() {
        var start = new Date(2018, 1, 1, 11, 30, 0)
        var editor = createTemporaryObject(editorComponent, testCase, {editMode: false, start: start})
        verify(editor)
        var startDate = findChild(editor, "startDate");
        compare(startDate.dateTime, start)

        var endDate = findChild(editor, "endDate");
        compare(endDate.dateTime, start)
    }

    function test_3loadControllerDates() {
        var start = new Date(2018, 1, 1, 11, 30, 0)
        var end = new Date(2018, 1, 1, 12, 0, 0)
        var controller = createTemporaryObject(controllerComponent, testCase, {start: start, end: end, allDay: false})
        var editor = createTemporaryObject(editorComponent, testCase, {editMode: true, controller: controller})
        verify(editor)

        var startDate = findChild(editor, "startDate");
        compare(startDate.dateTime, start)

        var endDate = findChild(editor, "endDate");
        compare(endDate.dateTime, end)
    }

    function test_4roundLoadedDates() {
        var start = new Date(2018, 1, 1, 11, 33, 0)
        var startRounded = new Date(2018, 1, 1, 11, 30, 0)
        var end = new Date(2018, 1, 1, 11, 58, 0)
        var endRounded = new Date(2018, 1, 1, 12, 0, 0)
        var controller = createTemporaryObject(controllerComponent, testCase, {start: start, end: end, allDay: false})
        var editor = createTemporaryObject(editorComponent, testCase, {editMode: true, controller: controller})
        verify(editor)

        var startDate = findChild(editor, "startDate");
        compare(startDate.dateTime, startRounded)

        var endDate = findChild(editor, "endDate");
        compare(endDate.dateTime, endRounded)
    }
}
