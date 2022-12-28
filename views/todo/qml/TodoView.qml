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
import QtQuick.Controls 2.3

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    property var controller: null

    signal done()

    onControllerChanged: {
        //Wait for a controller to be set before we add a todo-view
        if (controller) {
            stackView.push(eventDetails, StackView.Immediate)
        }
    }

    function edit() {
        var item = stackView.push(editor, StackView.Immediate)
        item.forceActiveFocus()
    }

    StackView {
        id: stackView
        anchors.fill: parent
        clip: true
        visible: controller
    }

    Component {
        id: eventDetails
        Rectangle {
            color: Kube.Colors.paperWhite

            ColumnLayout {
                id: contentLayout
                anchors {
                    fill: parent
                    margins: Kube.Units.largeSpacing
                }

                spacing: Kube.Units.smallSpacing

                Kube.Heading {
                    Layout.fillWidth: true
                    text: controller.summary
                }

                Kube.SelectableLabel {
                    visible: !isNaN(controller.due)
                    text: qsTr("Due on ") + controller.due.toLocaleString(Qt.locale(), "dd. MMMM")
                    opacity: 0.75
                }

                Kube.SelectableLabel {
                    visible: !isNaN(controller.start)
                    text: qsTr("Start on ") + controller.start.toLocaleString(Qt.locale(), "dd. MMMM")
                    opacity: 0.75
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Kube.Colors.textColor
                    opacity: 0.5
                }

                Kube.ScrollableTextArea {
                    id: textArea
                    Layout.preferredHeight: implicitHeight
                    Layout.maximumHeight: implicitHeight
                    Layout.fillWidth: true
                    text: Kube.HtmlUtils.toHtml(controller.description)
                    textFormat: Kube.TextArea.RichText
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
                    id: button
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    text: "+ " + qsTr("Add subtodo")
                    textColor: Kube.Colors.highlightColor
                    focus: true
                    onClicked: {
                        Kube.Fabric.postMessage(Kube.Messages.todoEditor, {"parentUid": controller.uid})
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                RowLayout {
                    width: parent.width
                    Kube.Button {
                        text: qsTr("Remove")
                        onClicked: {
                            root.controller.remove()
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Kube.Button {
                        text: qsTr("Edit")
                        onClicked: root.edit()
                    }

                }
            }
        }
    }

    Component {
        id: editor
        TodoEditor {
            controller: root.controller
            editMode: true
            onDone: {
                //Reload
                root.controller.todo = root.controller.todo
                stackView.pop(StackView.Immediate)
            }
        }
    }
}
