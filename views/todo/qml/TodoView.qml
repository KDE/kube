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

                Flickable {
                    id: flickable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.horizontal: Kube.ScrollBar {  }
                    contentHeight: textArea.height
                    contentWidth: textArea.width
                    clip: true
                    Kube.TextArea {
                        id: textArea
                        width: flickable.width
                        text: Kube.HtmlUtils.toHtml(controller.description)
                        textFormat: Kube.TextArea.RichText
                    }
                    Kube.ScrollHelper {
                        anchors.fill: parent
                        flickable: flickable
                    }
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
