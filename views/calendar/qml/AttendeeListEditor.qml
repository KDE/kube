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

    implicitHeight: listView.implicitHeight + lineEdit.height

    ColumnLayout {
        anchors.fill: parent

        spacing: Kube.Units.smallSpacing

        Kube.ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Kube.Units.smallSpacing
            currentIndex: -1

            model: controller.model
            delegate: Kube.ListDelegate {
                height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 2 //smallSpacing for padding
                selectionEnabled: false

                Rectangle {
                    anchors.fill: parent
                    color: Kube.Colors.darkBackgroundColor
                    opacity: 0.7
                }

                RowLayout {
                    anchors {
                        fill: parent
                        leftMargin: Kube.Units.smallSpacing
                        rightMargin: Kube.Units.smallSpacing
                    }
                    spacing: Kube.Units.smallSpacing

                    Kube.Label {
                        Layout.maximumWidth: parent.width - statusLabel.width - iconButton.width - 4 * Kube.Units.smallSpacing
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

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    Kube.Label {
                        id: statusLabel
                        text: model.status == Kube.EventController.Accepted ? qsTr("Attending") : qsTr("Invited")
                        font.italic: true
                        font.pointSize: Kube.Units.smallFontSize
                        color: Kube.Colors.highlightedTextColor
                    }
                    Kube.IconButton {
                        id: iconButton
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
