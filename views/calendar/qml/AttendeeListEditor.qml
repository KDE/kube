/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
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


import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    property var controller
    property var completer

    property alias count: listView.count

    implicitHeight: flow.height + lineEdit.height
    height: implicitHeight

    Column {
        anchors.fill: parent

        spacing: Kube.Units.smallSpacing

        Flow {
            id: flow
            anchors {
                left: parent.left
                right: parent.right
            }
            spacing: Kube.Units.smallSpacing


            Repeater {
                id: listView

                model: controller.model
                delegate: Item {
                    height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 2 //smallSpacing for padding
                    width: label.width + status.width + Kube.Units.gridUnit + Kube.Units.largeSpacing + Kube.Units.smallSpacing * 3

                    Rectangle {
                        anchors.fill: parent
                        color: Kube.Colors.darkBackgroundColor
                        opacity: 0.7
                    }

                    Kube.Label {
                        id: label
                        anchors {
                            left: parent.left
                            leftMargin: Kube.Units.smallSpacing
                            verticalCenter: parent.verticalCenter
                        }
                        text: model.name
                        elide: Text.ElideRight
                        color: Kube.Colors.highlightedTextColor
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                        ToolTip.visible: mouseArea.containsMouse
                        ToolTip.text: text
                    }

                    Kube.Label {
                        id: status
                        anchors {
                            left: label.right
                            leftMargin: Kube.Units.largeSpacing
                            verticalCenter: parent.verticalCenter
                        }
                        text: model.status == Kube.EventController.Accepted ? qsTr("Attending") : qsTr("Invited")
                        font.italic: true
                        font.pointSize: Kube.Units.smallFontSize
                        color: Kube.Colors.highlightedTextColor
                    }
                    Kube.IconButton {
                        anchors {
                            left: status.right
                            leftMargin: Kube.Units.smallSpacing
                            verticalCenter: parent.verticalCenter
                        }
                        height: Kube.Units.gridUnit
                        width: height
                        onClicked: root.controller.remove(model.id)
                        padding: 0
                        iconName: Kube.Icons.remove
                    }
                }
            }
        }

        FocusScope {
            height: Kube.Units.gridUnit +  Kube.Units.smallSpacing * 2
            width: parent.width
            focus: true

            Kube.TextButton {
                id: button
                text: "+ " + qsTr("Add attendee")
                textColor: Kube.Colors.highlightColor
                focus: true
                onClicked: {
                    lineEdit.visible = true
                    lineEdit.forceActiveFocus()
                }
            }

            Kube.AutocompleteLineEdit {
                id: lineEdit
                anchors {
                    left: parent.left
                    right: parent.right
                }
                visible: false

                placeholderText: "+ " + qsTr("Add attendee")
                model: root.completer.model
                onSearchTermChanged: root.completer.searchString = searchTerm
                onAccepted: {
                    root.controller.add({name: text});
                    clear()
                    visible = false
                    button.forceActiveFocus(Qt.TabFocusReason)
                }
                onAborted: {
                    clear()
                    visible = false
                    button.forceActiveFocus(Qt.TabFocusReason)
                }
            }
        }
    }
}
