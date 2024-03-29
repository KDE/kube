/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
 *  Copyright (C) 2019 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.4
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube
import "dateutils.js" as DateUtils

Item {
    id: root

    property bool editMode: false
    property date start: new Date()
    property bool allDay: false
    property var controller: Kube.EventController { allDay: root.allDay }
    property var accountId: null

    signal done()

    function discard() {
        controller.reload()
        root.done()
    }

    implicitWidth: contentLayout.implicitWidth + 2 * Kube.Units.largeSpacing
    implicitHeight: contentLayout.implicitHeight + buttons.implicitHeight + 2 * Kube.Units.largeSpacing

    states: [
    State {
        name: "edit"
        PropertyChanges { target: deleteButton; visible: true }
        PropertyChanges { target: abortButton; visible: false }
        PropertyChanges { target: saveButton; visible: true }
        PropertyChanges { target: discardButton; visible: true }
        PropertyChanges { target: createButton; visible: false }
        PropertyChanges { target: calendarSelector; visible: false }
    },
    State {
        name: "new"
        PropertyChanges { target: deleteButton; visible: false }
        PropertyChanges { target: abortButton; visible: true }
        PropertyChanges { target: saveButton; visible: false }
        PropertyChanges { target: discardButton; visible: false }
        PropertyChanges { target: createButton; visible: true }
        PropertyChanges { target: calendarSelector; visible: true }
    }
    ]

    state: editMode ? "edit" : "new"

    ColumnLayout {
        id: contentLayout

        anchors {
            fill: parent
            margins: Kube.Units.largeSpacing
        }

        spacing: Kube.Units.largeSpacing

        ColumnLayout {

            spacing: Kube.Units.largeSpacing

            Kube.HeaderField {
                id: titleEdit
                Layout.fillWidth: true
                placeholderText: qsTr("Event Title")
                text: controller.summary
                onTextChanged: controller.summary = text
            }

            DateRangeChooser {
                id: dateAndTimeChooser
                Layout.fillWidth: true
                initialStart: root.editMode ? controller.start : root.start
                initialEnd: root.editMode ? controller.end : DateUtils.addMinutesToDate(root.start, 30)
                allDay: controller.allDay
                onStartChanged: controller.start = start
                onEndChanged: controller.end = end
                onAllDayChanged: controller.allDay = allDay
            }

            ColumnLayout {
                spacing: Kube.Units.smallSpacing
                Layout.fillWidth: true

                Kube.TextField {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Location")
                    text: controller.location
                    onTextChanged: controller.location = text
                }

                RowLayout {
                    visible: attendees.count
                    Layout.maximumWidth: parent.width
                    Layout.fillWidth: true
                    Kube.Label {
                        id: fromLabel
                        text: qsTr("Organizer:")
                    }

                    Kube.ComboBox {
                        id: identityCombo
                        objectName: "identityCombo"

                        width: parent.width - Kube.Units.largeSpacing * 2

                        model: root.controller.identitySelector.model
                        textRole: "address"
                        Layout.fillWidth: true
                        //A regular binding is not enough in this case, we have to use the Binding element
                        Binding { target: identityCombo; property: "currentIndex"; value: root.controller.identitySelector.currentIndex }
                        onCurrentIndexChanged: {
                            root.controller.identitySelector.currentIndex = currentIndex
                        }
                    }
                }

                AttendeeListEditor {
                    id: attendees
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    focus: true
                    activeFocusOnTab: true
                    controller: root.controller.attendees
                    completer: root.controller.attendeeCompleter
                }

                Kube.SeparatorLine {
                    Layout.fillWidth: true
                }

                Kube.TextEditor {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: Kube.Units.gridUnit * 4

                    border.width: 0

                    placeholderText: "Description"
                    initialText: controller.description
                    onTextChanged: controller.description = text

                    Keys.onEscapePressed: calendarSelector.forceActiveFocus(Qt.TabFocusReason)
                }
            }
        }

        RowLayout {
            id: buttons

            spacing: Kube.Units.largeSpacing

            Kube.Button {
                id: deleteButton
                text: qsTr("Delete")
                onClicked: {
                    controller.remove()
                    root.done()
                }
            }

            Kube.Button {
                id: abortButton
                text: qsTr("Abort")
                onClicked: {
                    root.done()
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Kube.EntityComboBox {
                id: calendarSelector
                accountId: root.accountId
                type: "calendar"
                filter: {"contentTypes": "event", "enabled": true}
                onSelected: {
                    if (!root.editMode) {
                        controller.calendar = entity
                    }
                }
            }

            Kube.Button {
                id: discardButton
                enabled: controller.modified
                text: qsTr("Discard Changes")
                onClicked: root.discard()
            }

            Kube.PositiveButton {
                id: saveButton
                enabled: controller.modified
                text: qsTr("Save Changes")
                onClicked: {
                    controller.saveAction.execute()
                    root.done()
                }
            }

            Kube.PositiveButton {
                id: createButton
                text: qsTr("Create Event")
                onClicked: {
                    controller.saveAction.execute()
                    root.done()
                }
            }
        }
    }
}
