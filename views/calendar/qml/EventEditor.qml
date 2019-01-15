/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
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

Item {
    id: root

    property var controller: Kube.EventController {}
    property bool editMode: false
    property date start: new Date()

    signal done()

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

            Kube.TextField {
                id: titleEdit
                Layout.fillWidth: true
                placeholderText: qsTr("Event Title")
                text: controller.summary
                onTextChanged: controller.summary = text
            }

            ColumnLayout {
                id: dateAndTimeChooser

                spacing: Kube.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kube.Units.largeSpacing
                    DateTimeChooser {
                        id: startDate
                        objectName: "startDate"
                        enableTime: !controller.allDay
                        initialValue: root.editMode ? controller.start : root.start
                        onDateTimeChanged: controller.start = dateTime
                    }
                    Kube.Label {
                        text: qsTr("until")
                    }
                    DateTimeChooser {
                        id: endDate
                        objectName: "endDate"
                        enableTime: !controller.allDay
                        notBefore: startDate.dateTime
                        initialValue: root.editMode ? controller.end : startDate.dateTime
                        onDateTimeChanged: controller.end = dateTime
                    }
                }

                RowLayout {
                    spacing: Kube.Units.smallSpacing
                    Kube.CheckBox {
                        onClicked: {
                            checked: controller.allDay
                            onClicked: {
                                controller.allDay = !controller.allDay
                            }
                        }
                    }
                    Kube.Label {
                        text: qsTr("All day")
                    }
                }
            }

            ColumnLayout {
                spacing: Kube.Units.smallSpacing
                Layout.fillWidth: true
                //FIXME location doesn't exist yet
                // Kube.TextField {
                //     Layout.fillWidth: true
                //     placeholderText: qsTr("Location")
                //     text: controller.location
                //     onTextChanged: controller.location = text
                // }

                Kube.TextEditor {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: Kube.Units.gridUnit * 4

                    //TODO placeholderText: "Description"
                    text: controller.description
                    onTextChanged: controller.description = text
                }

                Kube.ComboBox {
                    id: calendarSelector
                    Layout.fillWidth: true

                    model: Kube.EntityModel {
                        id: calendarModel
                        type: "calendar"
                        roles: ["name"]
                    }
                    textRole: "name"
                    onActivated: {
                        controller.calendar = calendarModel.data(index).object
                    }
                }
            }
        }

        RowLayout {
            id: buttons

            spacing: Kube.Units.smallSpacing

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

            Kube.Button {
                id: discardButton
                text: qsTr("Discard Changes")
                onClicked: {
                    root.done()
                }
            }

            Kube.PositiveButton {
                id: saveButton
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
