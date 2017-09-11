/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    //Private properties
    property variant parentFolder: null
    property bool isDraft : false
    property bool isImportant : false
    property bool isTrash : false
    property bool isUnread : false
    property variant currentMail: null

    onCurrentMailChanged: Kube.Fabric.postMessage(Kube.Messages.mailSelection, {"mail":currentMail})

    Kube.Listener {
        filter: Kube.Messages.folderSelection
        onMessageReceived: {
            parentFolder = message.folder
            currentMail = null
        }
    }

    Kube.Listener {
        filter: Kube.Messages.search
        onMessageReceived: {
            mailListModel.filter = message.filterString
        }
    }

    Shortcut {
        sequence: StandardKey.Delete
        enabled: !isTrash
        onActivated: Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail":currentMail})
    }

    Kube.Label {
        anchors.centerIn: parent
        visible: listView.count === 0
        //TODO depending on whether we synchronized already or not the label should change.
        text: qsTr("Nothing here...")
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        Kube.TextField {
            Layout.fillWidth: true
            placeholderText: "Filter..."
            onTextChanged: mailListModel.filter = text
        }

        Kube.ListView {
            id: listView

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            focus: true

            //BEGIN keyboard nav
            onActiveFocusChanged: {
                if (activeFocus && currentIndex < 0) {
                    currentIndex = 0
                }
            }

            Keys.onDownPressed: {
                incrementCurrentIndex()
            }
            Keys.onUpPressed: {
                decrementCurrentIndex()
            }
            //END keyboard nav

            onCurrentItemChanged: {
                if (currentItem) {
                    root.currentMail = currentItem.currentData.mail;
                    root.isDraft = currentItem.currentData.draft;
                    root.isTrash = currentItem.currentData.trash;
                    root.isImportant = currentItem.currentData.important;
                    root.isUnread = currentItem.currentData.unread;
                }
            }

            model: Kube.MailListModel {
                id: mailListModel
                parentFolder: root.parentFolder
            }

            delegate: Kube.ListDelegate {
                id: delegateRoot
                //Required for D&D
                property var mail: model.mail

                width: listView.width - Kube.Units.smallSpacing
                height: Kube.Units.gridUnit * 5

                color: Kube.Colors.viewBackgroundColor
                border.color: Kube.Colors.backgroundColor
                border.width: 1

                states: [
                State {
                    name: "dnd"
                    when: mouseArea.drag.active

                    PropertyChanges {target: mouseArea; cursorShape: Qt.ClosedHandCursor}
                    PropertyChanges {target: delegateRoot; x: x; y: y}
                    PropertyChanges {target: delegateRoot; parent: root}
                    PropertyChanges {target: delegateRoot; opacity: 0.7}
                    PropertyChanges {target: delegateRoot; highlighted: true}
                }
                ]

                Drag.active: mouseArea.drag.active
                Drag.hotSpot.x: mouseArea.mouseX
                Drag.hotSpot.y: mouseArea.mouseY
                Drag.source: delegateRoot

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    drag.target: parent
                    onReleased: parent.Drag.drop()
                    onClicked: delegateRoot.clicked()
                }

                Item {
                    id: content

                    anchors {
                        fill: parent
                        margins: Kube.Units.smallSpacing
                    }
                    property color unreadColor: (model.unread && !delegateRoot.highlighted) ? Kube.Colors.highlightColor : delegateRoot.textColor

                    //TODO batch editing
                    //                 Kube.CheckBox {
                    //                     id: checkBox
                    //
                    //                     anchors.verticalCenter: parent.verticalCenter
                    //                     visible: (checked || delegateRoot.hovered) && !mouseArea.drag.active
                    //                     opacity: 0.9
                    //                 }

                    Column {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin:  Kube.Units.largeSpacing // + checkBox.width
                        }

                        Kube.Label{
                            id: subject
                            width: content.width - Kube.Units.gridUnit * 3
                            text: model.subject
                            color: content.unreadColor
                            maximumLineCount: 2
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                        }

                        Kube.Label {
                            id: sender
                            text: model.senderName
                            color: delegateRoot.textColor
                            font.italic: true
                            width: delegateRoot.width - Kube.Units.gridUnit * 3
                            elide: Text.ElideRight
                        }
                    }

                    Kube.Label {
                        id: date
                        anchors {
                            right: parent.right
                            bottom: parent.bottom
                        }
                        visible: !delegateRoot.hovered
                        text: Qt.formatDateTime(model.date, "dd MMM yyyy")
                        font.italic: true
                        color: Kube.Colors.disabledTextColor
                        font.pointSize: Kube.Units.tinyFontSize
                    }

                    Kube.Label {
                        id: threadCounter
                        anchors.right: parent.right
                        text: model.threadSize
                        color: content.unreadColor
                        visible: model.threadSize > 1
                    }
                }

                Row {
                    id: buttons

                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                        margins: Kube.Units.smallSpacing
                    }

                    visible: delegateRoot.hovered && !mouseArea.drag.active

                    spacing: Kube.Units.smallSpacing
                    opacity: 0.7

                    Kube.IconButton {
                        id: readButton
                        iconName: Kube.Icons.markAsRead
                        visible: model.unread
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": model.mail})
                        activeFocusOnTab: false
                    }
                    Kube.IconButton {
                        id: unreadButton
                        iconName: Kube.Icons.markAsUnread
                        visible: !model.unread
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.markAsUnread, {"mail": model.mail})
                        activeFocusOnTab: false
                    }

                    Kube.IconButton {
                        id: importantButton
                        iconName: Kube.Icons.markImportant
                        visible: !!model.mail
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.toggleImportant, {"mail": model.mail, "important": model.important})
                        activeFocusOnTab: false
                    }

                    Kube.IconButton {
                        id: deleteButton
                        iconName: Kube.Icons.moveToTrash
                        visible: !!model.mail
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": model.mail})
                        activeFocusOnTab: false
                    }

                    Kube.IconButton {
                        id: restoreButton
                        iconName: Kube.Icons.undo
                        visible: !!model.trash
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.restoreFromTrash, {"mail": model.mail})
                        activeFocusOnTab: false
                    }
                }
            }
        }
    }
}
