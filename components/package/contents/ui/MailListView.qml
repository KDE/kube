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

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework

Item {
    id: root
    property variant parentFolder
    property variant currentMail: null
    property bool isDraft : false
    property int currentIndex
    property string filterString: searchBar.text;

    onParentFolderChanged: {
        currentMail = null
    }

    ToolBar {
        id: toolbar

        width: parent.width

        Row {
            anchors.centerIn: parent

            spacing: Kirigami.Units.smallSpacing

            Controls.ToolButton {
                iconName: "mail-mark-unread-new"
                text: qsTr("Mark As Read")
                enabled: mailController.markAsReadAction.enabled
                tooltip: qsTr("mark mail as read")
                onClicked: {
                    mailController.markAsReadAction.execute()
                }
            }

            Controls.ToolButton {
                iconName: "mail-mark-important"
                text: qsTr("Mark Important")
                enabled: mailController.markAsImportantAction.enabled
                tooltip: qsTr("mark mail as important")
                onClicked: {
                    mailController.markAsImportantAction.execute()
                }
            }

            Controls.ToolButton {
                iconName: "edit-delete"
                text: qsTr("Delete Mail")
                enabled: mailController.moveToTrashAction.enabled
                tooltip: qsTr("delete email")
                onClicked: {
                    mailController.moveToTrashAction.execute()
                }
            }

            Controls.ToolButton {
                iconName: "edit-undo"
                text: qsTr("Restore Mail")
                enabled: mailController.restoreFromTrashAction.enabled
                tooltip: qsTr("restore email")
                onClicked: {
                    mailController.restoreFromTrashAction.execute()
                }
            }
        }
    }

    TextField {
        id: searchBar
        anchors.top: toolbar.bottom
        width: parent.width
        placeholderText: qsTr("Filter...")
    }

    Label {
        anchors.top: searchBar.bottom
        visible: listView.count === 0
        //TODO depending on whether we synchronized already or not the label should change.
        text: "Nothing here..."
    }

    ListView {
        id: listView

        anchors.top: searchBar.bottom

        width: parent.width
        height: parent.height - toolbar.height

        focus: true
        clip: true

        ScrollBar.vertical: ScrollBar{
            id: scrollbar
        }

        //BEGIN keyboard nav
        Keys.onDownPressed: {
            incrementCurrentIndex()
        }
        Keys.onUpPressed: {
            decrementCurrentIndex()
        }
        //END keyboard nav

        currentIndex: root.currentIndex
        onCurrentItemChanged: {
            root.currentMail = currentItem.currentData.domainObject;
            root.isDraft = currentItem.currentData.draft;
        }

        model: KubeFramework.MailListModel {
            parentFolder: root.parentFolder
            filter: root.filterString
        }

        delegate: Item {
            id: origin

            property variant currentData: model

            width: delegateRoot.width
            height: delegateRoot.height

            Item {
                id: delegateRoot

                property variant mail : model.domainObject

                width: scrollbar.visible ? listView.width - scrollbar.width : listView.width
                height: Kirigami.Units.gridUnit * 5

                states: [
                State {
                    name: "dnd"
                    when: mouseArea.drag.active

                    PropertyChanges {target: mouseArea; cursorShape: Qt.ClosedHandCursor}
                    PropertyChanges {target: delegateRoot; x: x; y:y}
                    PropertyChanges {target: delegateRoot; parent: root}

                    PropertyChanges {target: delegateRoot; opacity: 0.7}
                    PropertyChanges {target: background; color: Kirigami.Theme.highlightColor}
                    PropertyChanges {target: subject; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: sender; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: date; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: threadCounter; color: Kirigami.Theme.highlightedTextColor}
                },
                State {
                    name: "selected"
                    when: listView.currentIndex == index && !mouseArea.drag.active

                    PropertyChanges {target: background; color: Kirigami.Theme.highlightColor}
                    PropertyChanges {target: subject; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: sender; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: date; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: threadCounter; color: Kirigami.Theme.highlightedTextColor}
                },
                State {
                    name: "hovered"
                    when: mouseArea.containsMouse && !mouseArea.drag.active

                    PropertyChanges {target: background; color: Kirigami.Theme.buttonHoverColor; opacity: 0.7}
                    PropertyChanges {target: subject; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: sender; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: date; color: Kirigami.Theme.highlightedTextColor}
                    PropertyChanges {target: threadCounter; color: Kirigami.Theme.highlightedTextColor}
                }
                ]

                Drag.active: mouseArea.drag.active
                Drag.hotSpot.x: mouseArea.mouseX
                Drag.hotSpot.y: mouseArea.mouseY
                Drag.source: delegateRoot

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent

                    hoverEnabled: true
                    drag.target: parent

                    onClicked: {
                        listView.currentIndex = index
                    }
                    onReleased: parent.Drag.drop()
                }

                Rectangle {
                    id: background

                    anchors.fill: parent

                    color: Kirigami.Theme.viewBackgroundColor

                    border.color: Kirigami.Theme.backgroundColor
                    border.width: 1
                }

                Item {
                    id: content

                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                        margins: Kirigami.Units.smallSpacing
                    }

                    Column {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kirigami.Units.largeSpacing
                        }

                        Text{
                            id: subject

                            text: model.subject
                            color: model.unread ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

                            maximumLineCount: 2
                            width: content.width - Kirigami.Units.gridUnit * 3
                            wrapMode: Text.WrapAnywhere
                            elide: Text.ElideRight
                        }

                        Text {
                            id: sender

                            text: model.senderName
                            font.italic: true
                            color: Kirigami.Theme.textColor
                            width: delegateRoot.width - Kirigami.Units.gridUnit * 3
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        id: date

                        anchors {
                            right: parent.right
                            bottom: parent.bottom
                        }
                        text: Qt.formatDateTime(model.date, "dd MMM yyyy")
                        font.italic: true
                        color: Kirigami.Theme.disabledTextColor
                        font.pointSize: 9
                    }

                    Text {
                        id: threadCounter

                        anchors {
                            right: parent.right
                        }
                        text: model.threadSize
                        color: model.unread ?  Kirigami.Theme.highlightColor  : Kirigami.Theme.disabledTextColor
                        visible: model.threadSize > 1
                    }
                }
            }
        }
    }
}
