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
import QtQml 2.2 as QtQml

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework

Item {
    id: root
    property variant parentFolder
    property variant currentMail
    property bool isDraft : false
    property int currentIndex

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
                iconName: "mail-mark-unread"
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

    ListView {
        id: listView

        anchors.top: toolbar.bottom

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

        model: KubeFramework.MailListModel {
            parentFolder: root.parentFolder
        }

        delegate: Kirigami.AbstractListItem {
            id: mailListDelegate

            width: scrollbar.visible ? listView.width - scrollbar.width : listView.width
            height: Kirigami.Units.gridUnit * 4.5

            enabled: true
            supportsMouseEvents: true

            checked: listView.currentIndex == index
            onClicked:  {
                listView.currentIndex = model.index
            }

            //Content
            Item {
                width: parent.width
                height: parent.height

                QtQml.Binding {
                    target: root
                    property: "currentMail"
                    when: listView.currentIndex == index
                    value: model.domainObject
                }
                QtQml.Binding {
                    target: root
                    property: "isDraft"
                    when: listView.currentIndex == index
                    value: model.draft
                }

                //TODO implement bulk action
//                 CheckBox {
//                     id: checkBox
//
//                     anchors.verticalCenter: parent.verticalCenter
//
//                     visible: mailListDelegate.containsMouse == true || checked
//                 }

                Column {
                    id: mainContent

                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: Kirigami.Units.largeSpacing
                    }

                    Text{
                        text: model.subject
                        color: mailListDelegate.checked ? Kirigami.Theme.highlightedTextColor : model.unread ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

                        maximumLineCount: 2
                        width: mailListDelegate.width - Kirigami.Units.largeSpacing * 2 - unreadCounter.width
                        wrapMode: Text.Wrap
                        elide: Text.ElideRight
                    }

                    Text {
                        width: mailListDelegate.width - Kirigami.Units.largeSpacing * 2 - unreadCounter.width
                        text: model.senderName
                        font.italic: true
                        color: mailListDelegate.checked ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                        elide: Text.ElideRight
                    }
                }

                Text {
                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                    }
                    text: Qt.formatDateTime(model.date, "dd MMM yyyy")
                    font.italic: true
                    color: mailListDelegate.checked ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.disabledTextColor
                    font.pointSize: 9

                    //visible: mailListDelegate.containsMouse == false

                }

                Text {
                    id: unreadCounter

                    anchors {
                        right: parent.right
                    }

                    visible: model.threadSize > 1

                    font.italic: true
                    text: model.threadSize
                    color: mailListDelegate.checked ? Kirigami.Theme.highlightedTextColor : model.unread ? "#1d99f3" : Kirigami.Theme.disabledTextColor
                }

//                 Row {
//                     id: actionButtons
//
//                     anchors {
//                         right: parent.right
//                         bottom: parent.bottom
//                     }
//
//                     visible: mailListDelegate.containsMouse == true
//                     spacing: Kirigami.Units.smallSpacing
//
//                     Controls.ToolButton {
//                         iconName: "mail-mark-unread"
//                         enabled: mailController.markAsReadAction.enabled
//                         onClicked: {
//                             //mailController.markAsReadAction.execute()
//                         }
//                     }
//
//                     Controls.ToolButton {
//                         iconName: "mail-mark-important"
//                         enabled: mailController.markAsImportantAction.enabled
//                         onClicked: {
//                             //mailController.markAsImportantAction.execute()
//                         }
//                     }
//
//                     Controls.ToolButton {
//                         iconName: "edit-delete"
//                         enabled: mailController.moveToTrashAction.enabled
//                         onClicked: {
//                             //mailController.moveToTrashAction.execute()
//                         }
//                     }
//                 }
            }
        }
    }
}
