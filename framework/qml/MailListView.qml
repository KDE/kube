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

import QtQuick 2.9
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    //Private properties
    property var parentFolder: null
    property var currentMail: null
    property var currentAccount: null
    property bool showImportant: false
    property var filter: null
    property alias threaded: mailListModel.threaded

    onParentFolderChanged: {
        currentMail = null
    }

    Kube.Listener {
        filter: Kube.Messages.selectTopConversation
        onMessageReceived: {
            listView.currentIndex = 0
            listView.forceActiveFocus()
        }
    }

    Kube.Listener {
        filter: Kube.Messages.selectNextConversation
        onMessageReceived: {
            listView.incrementCurrentIndex()
            listView.forceActiveFocus()
        }
    }

    Kube.Listener {
        filter: Kube.Messages.selectPreviousConversation
        onMessageReceived: {
            listView.decrementCurrentIndex()
            listView.forceActiveFocus()
        }
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

        Kube.ListView {
            id: listView
            objectName: "listView"

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            focus: true

            onActiveFocusChanged: {
                if (activeFocus && currentIndex < 0) {
                    currentIndex = 0
                }
            }

            Keys.onPressed: {
                //Not implemented as a shortcut because we want it only to apply if we have the focus
                if (event.text == "d" || event.key == Qt.Key_Delete) {
                    Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": root.currentMail})
                } else if (event.text == "r") {
                    Kube.Fabric.postMessage(Kube.Messages.reply, {"mail": root.currentMail})
                } else if (event.text == "i") {
                    Kube.Fabric.postMessage(Kube.Messages.setImportant, {"mail": root.currentMail, "important": !currentItem.currentData.important})
                } else if (event.text == "u") {
                    Kube.Fabric.postMessage(Kube.Messages.markAsUnread, {"mail": root.currentMail})
                }
                if (event.key == Qt.Key_Home) {
                    listView.currentIndex = 0
                }
            }

            onCurrentItemChanged: {
                if (currentItem) {
                    var currentData = currentItem.currentData;
                    root.currentMail = currentData.mail;

                    if (currentData.mail && currentData.unread) {
                        Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": currentData.mail})
                    }
                }
            }

            model: Kube.MailListModel {
                id: mailListModel
                filter: {
                    "folder": root.parentFolder,
                    "important": root.showImportant,
                    "string": root.filter,
                    "account": root.currentAccount,
                    "headersOnly": true,
                    "fetchMails": false
                }
            }

            delegate: Kube.ListDelegate {
                id: delegateRoot
                //Required for D&D
                property var mail: model.mail
                property bool buttonsVisible: delegateRoot.hovered

                width: listView.availableWidth
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
                    PropertyChanges {target: delegateRoot; opacity: 0.2}
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
                    onReleased: {
                        var dropAction = parent.Drag.drop()
                        if (dropAction == Qt.MoveAction) {
                            parent.visible = false
                        }
                    }
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

                        function sameDay(date1, date2) {
                            return date1.getFullYear() == date2.getFullYear() && date1.getMonth() == date2.getMonth() && date1.getDate() == date2.getDate()
                        }

                        function formatDateTime(date) {
                            const today = new Date()
                            if (sameDay(date, today)) {
                                return Qt.formatDateTime(date, "hh:mm")
                            }
                            const lastWeekToday = today.getTime() - ((24*60*60*1000) * 7);
                            if (date.getTime() >= lastWeekToday) {
                                return Qt.formatDateTime(date, "ddd hh:mm")
                            }
                            return Qt.formatDateTime(date, "dd MMM yyyy")
                        }

                        visible: !delegateRoot.buttonsVisible
                        text: formatDateTime(model.date)
                        font.italic: true
                        color: delegateRoot.disabledTextColor
                        font.pointSize: Kube.Units.tinyFontSize
                    }

                    Kube.Label {
                        id: threadCounter
                        anchors {
                            right: parent.right
                            margins: Kube.Units.smallSpacing
                        }
                        text: model.threadSize
                        color: content.unreadColor
                        visible: model.threadSize > 1 && !delegateRoot.buttonsVisible

                    }
                }

                Kube.Icon {
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        margins: Kube.Units.smallSpacing
                    }

                    visible:  model.important && !delegateRoot.buttonsVisible && !mouseArea.drag.active
                    iconName: Kube.Icons.isImportant
                }

                Column {
                    id: buttons

                    anchors {
                        right: parent.right
                        margins: Kube.Units.smallSpacing
                        verticalCenter: parent.verticalCenter
                    }

                    visible: delegateRoot.buttonsVisible && !mouseArea.drag.active
                    opacity: 0.7

                    Kube.IconButton {
                        id: restoreButton
                        iconName: Kube.Icons.undo
                        visible: !!model.trash
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.restoreFromTrash, {"mail": model.mail})
                        activeFocusOnTab: false
                        tooltip: qsTr("Restore from trash")
                    }

                    Kube.IconButton {
                        id: readButton
                        iconName: Kube.Icons.markAsRead
                        visible: model.unread && !model.trash
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": model.mail})
                        tooltip: qsTr("Mark as read")
                    }
                    Kube.IconButton {
                        id: unreadButton
                        iconName: Kube.Icons.markAsUnread
                        visible: !model.unread && !model.trash
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.markAsUnread, {"mail": model.mail})
                        activeFocusOnTab: false
                        tooltip: qsTr("Mark as unread")
                    }

                    Kube.IconButton {
                        id: importantButton
                        iconName: model.important ? Kube.Icons.markImportant : Kube.Icons.markUnimportant
                        visible: !!model.mail
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.setImportant, {"mail": model.mail, "important": !model.important})
                        activeFocusOnTab: false
                        tooltip: qsTr("Mark as important")
                    }

                    Kube.IconButton {
                        id: deleteButton
                        objectName: "deleteButton"
                        iconName: Kube.Icons.moveToTrash
                        visible: !!model.mail
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": model.mail})
                        activeFocusOnTab: false
                        tooltip: qsTr("Move to trash")
                    }
                }
            }
        }
    }
}
