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

    property bool editMode: false
    property var controller: Kube.TodoController {
        complete: false
        doing: false
    }

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
                placeholderText: qsTr("Todo Title")
                text: controller.summary
                onTextChanged: controller.summary = text
            }

            // ColumnLayout {
            //     id: dateAndTimeChooser

            //     spacing: Kube.Units.smallSpacing

                // RowLayout {
                //     Layout.fillWidth: true
                //     spacing: Kube.Units.largeSpacing
                //     Kube.CheckBox {
                //         checked: !isNaN(controller.due)
                //         // onCheckedChanged: {
                //         //     if (controller.important != checked) {
                //         //         controller.important = checked
                //         //     }
                //         // }
                //     }
                //     Kube.Label {
                //         text: qsTr("Due")
                //     }
                //     DateTimeChooser {
                //         enableTime: false
                //         // notBefore: startDate.dateTime
                //         // initialValue: root.editMode ? controller.end : startDate.dateTime
                //         onDateTimeChanged: controller.dudue dateTime
                //     }
                // }

                // RowLayout {
                //     spacing: Kube.Units.smallSpacing
                //     Kube.CheckBox {
                //         checked: controller.important
                //         onCheckedChanged: {
                //             if (controller.important != checked) {
                //                 controller.important = checked
                //             }
                //         }
                //     }
                //     Kube.Label {
                //         text: qsTr("Important")
                //     }
                // }
            // }

            ColumnLayout {
                spacing: Kube.Units.smallSpacing
                Layout.fillWidth: true

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

                    onCurrentIndexChanged: {
                        if (currentIndex >= 0) {
                            controller.calendar = calendarModel.data(currentIndex).object
                        }
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
                text: qsTr("Create Todo")
                onClicked: {
                    controller.saveAction.execute()
                    root.done()
                }
            }
        }
    }
}
