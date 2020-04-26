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
    property bool encrypt: false
    property alias label: heading.text

    implicitHeight: heading.height + listView.height + lineEdit.height + 2 * Kube.Units.smallSpacing
    height: implicitHeight

    DropArea {
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            color: Kube.Colors.highlightColor
            opacity: 0.3
            visible: parent.containsDrag
        }

        onDropped: {
            drop.accept(Qt.MoveAction)
            root.controller.add({name: drop.source.name})
        }
    }

    Column {
        anchors.fill: parent

        spacing: Kube.Units.smallSpacing

        Kube.Label {
            id: heading
        }

        ListView {
            id: listView
            anchors {
                left: parent.left
                right: parent.right
            }
            contentHeight: contentItem.childrenRect.height
            height: contentHeight
            spacing: Kube.Units.smallSpacing
            model: controller.model
            delegate: Rectangle {
                id: delegateRoot
                property var recipientId: model.id
                property var name: model.name
                height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 2 //smallSpacing for padding
                width: parent.width
                color: Kube.Colors.buttonColor

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    drag.target: parent
                    drag.filterChildren: true
                    drag.axis: "YAxis"
                    onReleased: {
                        if (parent.Drag.drop() == Qt.MoveAction) {
                            root.controller.remove(recipientId)
                        }
                    }
                }

                states: [
                    State {
                        name: "dnd"
                        when: mouseArea.drag.active

                        PropertyChanges {target: mouseArea; cursorShape: Qt.ClosedHandCursor}
                        PropertyChanges {target: delegateRoot; opacity: 0.5}
                        ParentChange {target: delegateRoot; parent: recipients; x: x; y: y}
                    }
                ]

                Drag.active: mouseArea.drag.active
                Drag.hotSpot.x: mouseArea.mouseX
                Drag.hotSpot.y: mouseArea.mouseY
                Drag.source: delegateRoot

                Kube.Label {
                    id: label
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        right: keyIcon.left
                        margins: Kube.Units.smallSpacing
                    }
                    text: model.name
                    elide: Text.ElideRight
                    ToolTip.visible: mouseArea.containsMouse && !mouseArea.drag.active
                    ToolTip.text: text
                }
                Kube.IconButton {
                    id: keyIcon
                    anchors {
                        verticalCenter: parent.verticalCenter
                        right: removeButton.left
                    }
                    height: Kube.Units.gridUnit
                    width: visible ? height : 0
                    padding: 0
                    visible: root.encrypt && !model.fetching
                    iconName: model.keyFound ? Kube.Icons.secure: (hovered ? Kube.Icons.save : Kube.Icons.insecure)
                    enabled: !model.keyFound
                    tooltip: qsTr("Fetch key from keyserver")
                    onClicked: root.controller.fetchKeys(model.id, model.name)
                }
                Kube.Icon {
                    visible: model.fetching
                    anchors {
                        verticalCenter: parent.verticalCenter
                        right: removeButton.left
                    }
                    height: Kube.Units.gridUnit
                    iconName: Kube.Icons.busy
                }
                Kube.IconButton {
                    id: removeButton
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        margins: Kube.Units.smallSpacing
                    }
                    height: Kube.Units.gridUnit
                    width: height
                    onClicked: root.controller.remove(model.id)
                    padding: 0
                    iconName: Kube.Icons.remove
                }

            }
        }

        FocusScope {
            height: Kube.Units.gridUnit +  Kube.Units.smallSpacing * 2
            width: parent.width
            focus: true

            Kube.TextButton {
                id: button
                text: "+ " + qsTr("Add recipient")
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

                placeholderText: "+ " + qsTr("Add recipient")
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
