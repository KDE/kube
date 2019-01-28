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


import QtQuick 2.9
import QtQuick.Controls 1.3 as Controls1
import QtQuick.Controls 2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.View {
    id: root
    property alias currentAccount: accountSwitcher.currentAccount
    // property variant currentFolder: null

    //We have to hardcode because all the mapToItem/mapFromItem functions are garbage
    // searchArea: Qt.rect(ApplicationWindow.window.sidebarWidth + mailListView.parent.x, 0, (mailView.x + mailView.width) - mailListView.parent.x, (mailView.y + mailView.height) - mailListView.y)

    onFilterChanged: {
        mailListView.filter = filter
        Kube.Fabric.postMessage(Kube.Messages.searchString, {"searchString": filter})
    }

    onRefresh: {
        // if (!!root.currentFolder) {
        //     Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": root.currentFolder});
        //     Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": root.currentAccount, "type": "folder"})
        // } else {
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": root.currentAccount})
        // }
    }

    Kube.Listener {
        filter: Kube.Messages.search
        onMessageReceived: root.triggerSearch()
    }

    helpViewComponent: Kube.HelpPopup {
        ListModel {
            ListElement { description: qsTr("Jump to next thread:"); shortcut: "j" }
            ListElement { description: qsTr("Jump to previous thread:"); shortcut: "k" }
            ListElement { description: qsTr("Jump to next message:"); shortcut: "n" }
            ListElement { description: qsTr("Jump to previous message:"); shortcut: "p" }
            ListElement { description: qsTr("Jump to next folder:"); shortcut: "f,n" }
            ListElement { description: qsTr("Jump to previous previous folder:"); shortcut: "f,p" }
            ListElement { description: qsTr("Compose new message:"); shortcut: "c" }
            ListElement { description: qsTr("Reply to the currently focused message:"); shortcut: "r" }
            ListElement { description: qsTr("Delete the currently focused message:"); shortcut: "d" }
            ListElement { description: qsTr("Mark the currently focused message as important:"); shortcut: "i" }
            ListElement { description: qsTr("Mark the currently focused message as unread:"); shortcut: "u" }
            ListElement { description: qsTr("Show this help text:"); shortcut: "?" }
        }
    }

    Shortcut {
        sequences: ['j']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectNextConversation, {})
    }
    Shortcut {
        sequences: ['k']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectPreviousConversation, {})
    }
    Shortcut {
        sequences: ['Shift+J']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.scrollConversationDown, {})
    }
    Shortcut {
        sequences: ['Shift+K']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.scrollConversationUp, {})
    }
    Shortcut {
        sequences: ['n']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectNextMessage, {})
    }
    Shortcut {
        sequences: ['p']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectPreviousMessage, {})
    }
    Shortcut {
        sequences: ['f,n']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectNextFolder, {})
    }
    Shortcut {
        sequences: ['f,p']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectPreviousFolder, {})
    }
    Shortcut {
        sequences: ['c']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.compose, {})
    }
    Shortcut {
        sequence: "?"
        onActivated: root.showHelp()
    }


    Controls1.SplitView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Rectangle {
            width: Kube.Units.gridUnit * 10
            Layout.fillHeight: parent.height
            color: Kube.Colors.darkBackgroundColor

            Kube.PositiveButton {
                id: newMailButton
                objectName: "newMailButton"

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                focus: true
                text: qsTr("New Email")
                onClicked: Kube.Fabric.postMessage(Kube.Messages.compose, {})
            }

            Kube.InlineAccountSwitcher {
                id: accountSwitcher
                activeFocusOnTab: true
                anchors {
                    top: newMailButton.bottom
                    topMargin: Kube.Units.largeSpacing
                    bottom: statusBarContainer.top
                    left: newMailButton.left
                    right: parent.right
                }
                delegate: Kube.ListView {
                    id: listView
                    spacing: Kube.Units.smallSpacing
                    model: Kube.CheckableEntityModel {
                        id: calendarModel
                        type: "calendar"
                        roles: ["name", "color", "enabled"]
                        sortRole: "name"
                        accountId: accountSwitcher.accountId
                        // checkedEntities: calendarFilterCollector
                    }
                    delegate: Kube.ListDelegate {
                        id: delegate
                        width: listView.availableWidth
                        height: Kube.Units.gridUnit
                        hoverEnabled: true
                        background: Item {}
                        RowLayout {
                            anchors.fill: parent
                            spacing: Kube.Units.smallSpacing
                            Kube.CheckBox {
                                id: checkBox
                                opacity: 0.9
                                checked: model.checked || model.enabled
                                onCheckedChanged: {
                                    model.checked = checked
                                    model.enabled = checked
                                }

                                indicator: Rectangle {
                                    width: Kube.Units.gridUnit * 0.8
                                    height: Kube.Units.gridUnit * 0.8

                                    color: model.color

                                    Rectangle {
                                        id: highlight
                                        anchors.fill: parent
                                        visible: checkBox.hovered || checkBox.visualFocus
                                        color: Kube.Colors.highlightColor
                                        opacity: 0.4
                                    }

                                    Kube.Icon {
                                        anchors.centerIn: parent
                                        height: Kube.Units.gridUnit
                                        width: Kube.Units.gridUnit
                                        visible: checkBox.checked
                                        iconName: Kube.Icons.checkbox_inverted
                                    }
                                }

                            }
                            Kube.Label {
                                id: label
                                Layout.fillWidth: true
                                text: model.name
                                color: Kube.Colors.highlightedTextColor
                                elide: Text.ElideLeft
                                clip: true
                            }
                            ToolTip {
                                id: toolTipItem
                                visible: delegate.hovered && label.truncated
                                text: label.text
                            }
                        }
                    }
                }
            }

            Item {
                id: statusBarContainer
                anchors {
                    topMargin: Kube.Units.smallSpacing
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                height: childrenRect.height

                Rectangle {
                    id: border
                    visible: statusBar.visible
                    anchors {
                        right: parent.right
                        left: parent.left
                        margins: Kube.Units.smallSpacing
                    }
                    height: 1
                    color: Kube.Colors.viewBackgroundColor
                    opacity: 0.3
                }
                Kube.StatusBar {
                    id: statusBar
                    accountId: root.currentAccount
                    height: Kube.Units.gridUnit * 2
                    anchors {
                        top: border.bottom
                        left: statusBarContainer.left
                        right: statusBarContainer.right
                    }
                }
            }
        }

        Rectangle {
            width: Kube.Units.gridUnit * 18
            Layout.fillHeight: parent.height

            color: "transparent"
            border.width: 1
            border.color: Kube.Colors.buttonColor

            Kube.ListView {
                id: todoView
                anchors.fill: parent
                spacing: Kube.Units.smallSpacing
                Layout.minimumWidth: Kube.Units.gridUnit * 10
                model: Kube.EntityModel {
                    type: "todo"
                    roles: ["summary"]
                    sortRole: "summary"
                    accountId: accountSwitcher.accountId
                }
                delegate: Kube.ListDelegate {
                    id: delegate
                    width: ListView.view.availableWidth
                    height: Kube.Units.gridUnit * 2
                    hoverEnabled: true
                    RowLayout {
                        anchors.fill: parent
                        spacing: Kube.Units.smallSpacing
                        Kube.Label{
                            id: label
                            Layout.fillWidth: true
                            text: model.summary
                            color: delegate.textColor
                            maximumLineCount: 2
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                        }
                        ToolTip {
                            id: toolTipItem
                            visible: delegate.hovered && label.truncated
                            text: label.text
                        }
                    }
                }
            }
        }

    }

}
