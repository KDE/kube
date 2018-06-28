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
    property variant parentFolder: null
    property bool isDraft : false
    property bool isImportant : false
    property bool isTrash : false
    property bool isUnread : false
    property variant currentMail: null
    property bool showFilter: false
    property string filter: null

    onFilterChanged: {
        Kube.Fabric.postMessage(Kube.Messages.searchString, {"searchString": filter})
    }

    onParentFolderChanged: {
        currentMail = null
        filterField.clearSearch()
    }
    onShowFilterChanged: {
        find.forceActiveFocus()
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

        Rectangle {
            id: filterField
            Layout.fillWidth: true
            height: Kube.Units.gridUnit * 2
            color: Kube.Colors.buttonColor
            visible: root.showFilter

            function clearSearch() {
                root.showFilter = false
                find.text = ""
                root.filter = ""
            }

            RowLayout {
                anchors {
                    verticalCenter: parent.verticalCenter
                }

                width: parent.width - Kube.Units.smallSpacing
                spacing: 0

                Kube.IconButton {
                    iconName: Kube.Icons.remove
                    activeFocusOnTab: visible
                    onClicked: filterField.clearSearch()
                }

                Kube.TextField {
                    id: find
                    Layout.fillWidth: true
                    placeholderText: qsTr("Filter...")
                    onTextChanged: root.filter = text
                    activeFocusOnTab: visible
                    focus: visible
                    Keys.onEscapePressed: filterField.clearSearch()
                }
            }
        }

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
                if (event.text == "d") {
                    Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": root.currentMail})
                } else if (event.text == "r") {
                    Kube.Fabric.postMessage(Kube.Messages.reply, {"mail": root.currentMail})
                }
            }

            onCurrentItemChanged: {
                if (currentItem) {
                    var currentData = currentItem.currentData;
                    root.currentMail = currentData.mail;
                    root.isDraft = currentData.draft;
                    root.isTrash = currentData.trash;
                    root.isImportant = currentData.important;
                    root.isUnread = currentData.unread;

                    if (currentData.mail && currentData.unread) {
                        Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": currentData.mail})
                    }
                }
            }

            model: Kube.MailListModel {
                id: mailListModel
                parentFolder: root.parentFolder
                filter: root.filter
            }

            delegate: Kube.ListDelegate {
                id: delegateRoot
                //Required for D&D
                property var mail: model.mail

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
                        iconName: model.important ? Kube.Icons.markImportant : Kube.Icons.markUnimportant
                        visible: !!model.mail
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.setImportant, {"mail": model.mail, "important": !model.important})
                        activeFocusOnTab: false
                    }

                    Kube.IconButton {
                        id: deleteButton
                        objectName: "deleteButton"
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
