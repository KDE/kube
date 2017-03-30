/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
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
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

import QtQuick.Controls 2.0 as Controls2
import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.actions 1.0 as KubeAction
import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.notifications 1.0 as KubeNotifications
import org.kube.framework.accounts 1.0 as KubeAccountsFramework
import org.kube.components 1.0 as KubeComponents
import org.kube.components.accounts 1.0 as KubeAccounts
import org.kube.components.theme 1.0 as KubeTheme

Controls2.ApplicationWindow {
    id: app

    //FIXME remove fixed pixel hight
    //for now just convinience during testing
    height: 1080 * 0.8
    width: 1920  * 0.8

    visible: true

    KubeNotifications.NotificationHandler {
        id: notificationHandler
        function handler(n) {
            console.warn("We got a notification: ", n.message)
            if (n.type == KubeNotifications.Notification.Warning) {
                console.warn("And it's a warning!", n.type)
            }
            notificationPopup.notify(n.message);
        }
    }

    //BEGIN Actions
    KubeAction.Context {
        id: maillistcontext
        property variant mail
        property bool isDraft
        mail: mailListView.currentMail
        isDraft: mailListView.isDraft
    }

    KubeAction.Action {
        id: replyAction
        actionId: "org.kde.kube.actions.reply"
        context: maillistcontext
    }
    //END Actions

    //BEGIN ActionHandler
    KubeAction.ActionHandler {
        actionId: "org.kde.kube.actions.reply"
        function isReady(context) {
            return context.mail ? true : false;
        }

        function handler(context) {
            composer.loadMessage(context.mail, false)
            composer.open()
        }
    }

    KubeAction.ActionHandler {
        actionId: "org.kde.kube.actions.edit"
        function isReady(context) {
            return context.mail && context.isDraft;
        }
        function handler(context) {
            composer.loadMessage(context.mail, true)
            composer.open()
        }
    }
    //END ActionHandler

    //Controller
    KubeFramework.MailController {
        id: mailController
        Binding on threadLeader {
            //!! checks for the availability of the type
            when: !!mailListView.currentMail
            value: mailListView.currentMail
        }
    }

    KubeFramework.FolderController {
        id: folderController
        Binding on folder {
            //!! checks for the availability of the type
            when: !!folderListView.currentFolder
            value: folderListView.currentFolder
        }
    }

    //Model
    KubeAccountsFramework.AccountsModel {
        id: currentAccountModel
        accountId: accountSwitcher.accountId
    }

    //BEGIN Shortcuts
    Shortcut {
        sequence: StandardKey.Refresh
        onActivated: folderController.synchronizeAction.execute()
        enabled: folderController.synchronizeAction.enabled
    }
    Shortcut {
        sequence: StandardKey.Delete
        onActivated: mailController.moveToTrashAction.execute()
        enabled: mailController.moveToTrashAction.enabled
    }
    Shortcut {
        sequence: StandardKey.MoveToNextLine
        onActivated: mailListView.currentIndex++
    }
    Shortcut {
        sequence: StandardKey.MoveToPreviousLine
        onActivated: mailListView.currentIndex--
    }
    //END Shortcuts

    //BEGIN background
    Rectangle {
        anchors.fill: parent

        color: KubeTheme.Colors.backgroundColor
    }
    //END background

    //BEGIN Main content
    SplitView {
        anchors {
            top: app.top
            left: app.left
        }

        height: app.height
        width: app.width

        Rectangle {
            width: Kirigami.Units.gridUnit * 10
            Layout.maximumWidth: app.width * 0.25
            Layout.minimumWidth: Kirigami.Units.gridUnit * 5

            color: KubeTheme.Colors.textColor

            Controls2.ToolBar {
                id: toolBar

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }

                RowLayout {
                    anchors.centerIn: parent

                    spacing: Kirigami.Units.largeSpacing

                    KubeComponents.AccountSwitcher {
                        id: accountSwitcher

                        iconName: "kdenlive-menu"
                        height: Kirigami.Units.gridUnit * 1.5
                        width: height
                    }

                    ToolButton {
                        iconName: "user"
                        height: Kirigami.Units.gridUnit * 1.5
                        width: height

                        onClicked: {
                            people.open()
                        }
                    }

                    ToolButton {
                        iconName: "search"
                        height: Kirigami.Units.gridUnit * 1.5
                        width: height

                        onClicked: {
                            search.open()
                        }
                    }
                }
            }

            Rectangle {
                id: newMailButton

                anchors {
                    top: toolBar.bottom
                    left: parent.left
                    right: parent.right
                    margins: Kirigami.Units.largeSpacing
                }

                color: KubeTheme.Colors.positveColor
                clip: true

                height: Kirigami.Units.gridUnit * 1.5

                Text {
                    anchors.centerIn: parent

                    text: qsTr("New Email")
                    color: KubeTheme.Colors.highlightedTextColor
                }
                //iconName: "mail-message-new"
                //Controls2.Tooltip.text: "compose new email"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        composer.open()
                    }
                }
            }

            Item {
                id: accountName

                KubeFramework.FolderController {
                    id: accountNameFolderController
                    accountId: accountSwitcher.accountId
                }

                Menu {
                    id: contextMenu
                    title: "Edit"

                    MenuItem {
                        text: "Synchronize"
                        onTriggered: {
                            accountNameFolderController.synchronizeAction.execute()
                        }
                    }
                }

                anchors {
                    top: newMailButton.bottom
                    topMargin: Kirigami.Units.smallSpacing
                }

                width: parent.width
                height: Kirigami.Units.gridUnit * 2

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: {
                        contextMenu.popup()
                    }
                }

                Repeater {
                    model: currentAccountModel
                    Row {
                        anchors {
                            bottom: parent.bottom
                            left: parent.left
                            leftMargin: Kirigami.Units.smallSpacing
                        }
                        Layout.fillHeight: true

                        Text {
                            text: model.name
                            font.weight: Font.DemiBold
                            color: KubeTheme.Colors.highlightedTextColor
                        }

                        ToolButton {
                            id: statusIcon
                            visible: false
                            iconName: ""
                            enabled: false
                            states: [
                                State {
                                    name: "busy"; when: model.status == KubeAccountsFramework.AccountsModel.BusyStatus
                                    PropertyChanges { target: statusIcon; iconName: KubeTheme.Icons.busy; visible: true }
                                },
                                State {
                                    name: "error"; when: model.status == KubeAccountsFramework.AccountsModel.ErrorStatus
                                    PropertyChanges { target: statusIcon; iconName: KubeTheme.Icons.error; visible: true }
                                },
                                State {
                                    name: "checkmark"; when: model.status == KubeAccountsFramework.AccountsModel.ConnectedStatus
                                    PropertyChanges { target: statusIcon; iconName: KubeTheme.Icons.connected; visible: true }
                                },
                                State {
                                    name: "disconnected"; when: model.status == KubeAccountsFramework.AccountsModel.OfflineStatus
                                    PropertyChanges { target: statusIcon; iconName: KubeTheme.Icons.noNetworkConnection; visible: true }
                                }
                            ]
                        }
                    }
                }
            }

            KubeComponents.FolderListView {
                id: folderListView

                anchors {
                    top: accountName.bottom
                    topMargin: Kirigami.Units.smallSpacing
                    bottom: statusBar.top
                    left: parent.left
                    right: parent.right
                }

                focus: true
                accountId: accountSwitcher.accountId
            }

            Item {
                id: statusBar
                anchors {
                    topMargin: Kirigami.Units.smallSpacing
                    bottom: outbox.top
                    left: parent.left
                    right: parent.right
                }

                height: Kirigami.Units.gridUnit * 1

                Repeater {
                    model: currentAccountModel
                    Text {
                        id: statusText
                        anchors.centerIn: parent
                        visible: false
                        color: KubeTheme.Colors.textColor
                        states: [
                            State {
                                name: "disconnected"; when: model.status == KubeAccountsFramework.AccountsModel.OfflineStatus
                                PropertyChanges { target: statusText; text: "Offline"; visible: true }
                            }
                        ]
                    }
                }
            }

            KubeComponents.Outbox {
                id: outbox

                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
            }
        }

        KubeComponents.MailListView  {
            id: mailListView
            parentFolder: folderListView.currentFolder
            width: Kirigami.Units.gridUnit * 20
            height: parent.height
            Layout.maximumWidth: app.width * 0.4
            Layout.minimumWidth: Kirigami.Units.gridUnit * 10
            focus: true
        }

        KubeComponents.ConversationView {
            id: mailView
            mail: mailListView.currentMail
            Layout.fillWidth: true
        }
    }
    //END Main content

    //BEGIN Composer
    KubeComponents.FocusComposer {
        id: composer

        height: app.height * 0.85
        width: app.width * 0.85

        x: app.width * 0.075
        y: app.height * 0.075
    }
    //END Composer

    //BEGIN AccountWizard
    KubeAccounts.AccountWizard {
        id: accountWizard

        height: app.height * 0.85
        width: app.width * 0.85

        x: app.width * 0.075
        y: app.height * 0.075
    }
    //END AccountWizard

    //BEGIN Notification
    KubeComponents.Notification {
        id: notificationPopup

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
    }
    //END Notification

    //BEGIN Search
    Controls2.Popup {
        id: search

        width: app.width * 0.6
        height: Kirigami.Units.gridUnit * 3

        x: app.width * 0.2
        y: app.height * 0.2

        modal: true
        focus: true

        RowLayout {
            anchors.fill: parent

            Controls2.TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: "Search...   is not available in this beta"
            }

            Controls2.Button {
                text: "Go"

                onClicked: {
                    search.close()
                }
            }
        }
    }
    //END Search

    //BEGIN People
    KubeComponents.People {
        id: people

        height: app.height * 0.85
        width: app.width * 0.85

        x: app.width * 0.075
        y: app.height * 0.075

    }
    //END People
}
