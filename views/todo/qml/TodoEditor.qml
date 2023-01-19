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

import QtQuick 2.9
import QtQuick.Controls 2
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root

    property bool editMode: false
    property bool doing: false
    property var controller: Kube.TodoController {
        complete: false
        doing: root.doing
        parentUid: root.parentUid
    }
    property var accountId: null
    property var currentFolder: null
    property var parentUid: null

    Settings {
        id: settings
        category: "TodoEditor"
        property var lastUsedTodolist
    }

    signal done()

    function save() {
        controller.saveAction.execute()
        root.done()
    }

    function discard() {
        controller.reload()
        root.done()
    }

    implicitWidth: contentLayout.implicitWidth + 2 * Kube.Units.largeSpacing
    implicitHeight: contentLayout.implicitHeight + buttons.implicitHeight + 2 * Kube.Units.largeSpacing
    Keys.onEscapePressed: root.done()

    Shortcut {
        sequences: [StandardKey.Save]
        onActivated: root.save()
    }

    states: [
    State {
        name: "edit"
        PropertyChanges { target: deleteButton; visible: true }
        PropertyChanges { target: abortButton; visible: false }
        PropertyChanges { target: saveButton; visible: true }
        PropertyChanges { target: discardButton; visible: true }
        PropertyChanges { target: createButton; visible: false }
        PropertyChanges { target: calendarSelector; visible: false }
        PropertyChanges { target: subtodoButton; visible: true }
    },
    State {
        name: "new"
        PropertyChanges { target: deleteButton; visible: false }
        PropertyChanges { target: abortButton; visible: true }
        PropertyChanges { target: saveButton; visible: false }
        PropertyChanges { target: discardButton; visible: false }
        PropertyChanges { target: createButton; visible: true }
        PropertyChanges { target: calendarSelector; visible: true }
        PropertyChanges { target: subtodoButton; visible: false }
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

            spacing: Kube.Units.smallSpacing

            RowLayout {
                Layout.fillWidth: true
                spacing: Kube.Units.smallSpacing

                Kube.Label {
                    visible: calendarSelector.visible
                    text: qsTr("Create in:")
                }
                Kube.EntityComboBox {
                    id: calendarSelector
                    accountId: root.accountId
                    type: "calendar"
                    filter: {"contentTypes": "todo", "enabled": true}
                    initialSelection: root.currentFolder ? root.currentFolder : settings.lastUsedTodolist
                    onSelected: {
                        if (!root.editMode) {
                            controller.calendar = entity
                            settings.lastUsedTodolist = identifier
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Kube.CheckBox {
                    checked: controller.doing
                    onCheckedChanged: {
                        if (controller.doing != checked) {
                            controller.doing = checked
                        }
                    }
                }

                Kube.Label {
                    text: qsTr("Doing")
                }
            }

            Kube.HeaderField {
                id: titleEdit
                focus: true
                Layout.fillWidth: true
                placeholderText: qsTr("Todo Title")
                text: controller.summary
                onTextChanged: controller.summary = text
                Keys.onReturnPressed: {
                    controller.saveAction.execute()
                    root.done()
                }
            }

            Kube.TextEditor {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: Kube.Units.gridUnit * 4
                activeFocusOnTab: true

                border.width: 0

                placeholderText: "Description"
                initialText: controller.description
                onTextChanged: controller.description = text

                Keys.onEscapePressed: calendarSelector.forceActiveFocus(Qt.TabFocusReason)
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Kube.ListView {
                id: subTodoView
                width: parent.width
                Layout.preferredHeight: implicitHeight
                Layout.maximumHeight: implicitHeight
                Layout.fillWidth: true
                model: Kube.TodoModel {
                    id: todoModel
                    filter: {
                        "calendars": [controller.calendarId],
                        "parentUid": controller.uid
                    }
                }

                delegate: Kube.TodoListDelegate {
                    summary: model.summary
                    complete: model.complete
                    date: model.date
                    dueDate: model.dueDate
                    domainObject: model.domainObject

                    height: Kube.Units.gridUnit * 2 + 2 * Kube.Units.smallSpacing
                    subText: null
                    subtextVisible: false
                    currentDate: Kube.Context.currentDate
                    pickerActive: false
                }
            }

            Kube.TextButton {
                id: subtodoButton
                text: "+ " + qsTr("Add subtodo")
                textColor: Kube.Colors.highlightColor
                focus: true
                onClicked: {
                    Kube.Fabric.postMessage(Kube.Messages.todoEditor, {"parentUid": controller.uid})
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
                enabled: controller.modified
                text: qsTr("Discard Changes")
                onClicked: root.discard()
            }

            Kube.PositiveButton {
                id: saveButton
                enabled: controller.modified
                text: qsTr("Save Changes")
                onClicked: root.save()
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
