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
import QtQuick.Layouts 1.3
import QtQuick.Window 2.0

import QtQuick.Controls 2.0 as Controls2
import org.kube.framework 1.0 as Kube

Controls2.ApplicationWindow {
    id: app

    property int sidebarWidth: Kube.Units.gridUnit + Kube.Units.largeSpacing

    height: Screen.desktopAvailableHeight * 0.8
    width: Screen.desktopAvailableWidth * 0.8
    visible: true

    //Application default font
    font.family: Kube.Font.fontFamily

    //Application context
    property variant currentAccount
    onCurrentAccountChanged: {
        if (!!currentAccount) {
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": currentAccount})
        }
    }

    //Interval sync
    Timer {
        id: intervalSync
        //5min
        interval: 300000
        running: true
        repeat: true
        onTriggered: {
            if (kubeViews.currentItem && kubeViews.currentItem.refresh) {
                kubeViews.currentItem.refresh()
            }
        }
    }

    Kube.StartupCheck {
        id: startupCheck
    }

    Accounts {
    }

    //Listener
    Kube.Listener {
        filter: Kube.Messages.accountSelection
        onMessageReceived: app.currentAccount = message.accountId
    }

    Kube.Listener {
        filter: Kube.Messages.folderSelection
        onMessageReceived: Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": message.folder})

    }

    Kube.Listener {
        filter: Kube.Messages.notification
        onMessageReceived: {
            if (message.message) {
                notificationPopup.notify(message.message);
            }
        }
    }

    //BEGIN Shortcuts
    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }
    Shortcut {
        onActivated: Kube.Fabric.postMessage(Kube.Messages.search, {})
        sequence: StandardKey.Find
    }
    Shortcut {
        id: syncShortcut
        sequence: StandardKey.Refresh
        onActivated: {
            if (kubeViews.currentItem && kubeViews.currentItem.refresh) {
                kubeViews.currentItem.refresh()
            }
        }
    }
    //END Shortcuts

    //BEGIN background
    Rectangle {
        anchors.fill: parent

        color: Kube.Colors.backgroundColor
    }
    //END background

    //BEGIN Main content
    RowLayout  {
        id: mainContent
        spacing: 0
        anchors.fill: parent

        Rectangle {
            id: sideBar

            anchors {
                top: mainContent.top
                bottom: mainContent.bottom
            }
            width: app.sidebarWidth
            color: Kube.Colors.darkBackgroundColor

            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: Kube.Colors.viewBackgroundColor
                opacity: 0.3
            }

            Controls2.ButtonGroup { id: viewButtonGroup }

            Column {
                anchors {
                    top: parent.top
                    topMargin: Kube.Units.smallSpacing
                    horizontalCenter: parent.horizontalCenter
                }

                spacing: Kube.Units.largeSpacing - Kube.Units.smallSpacing

                Repeater {
                    model: Kube.ExtensionModel {
                        id: extensionModel
                        extensionPoint: "views"
                        sortOrder: ["search", "composer", "conversation", "people"]
                    }
                    Kube.IconButton {
                        id: button
                        iconName: model.icon
                        onClicked: kubeViews.showView(model.name)
                        activeFocusOnTab: true
                        checkable: true
                        Controls2.ButtonGroup.group: viewButtonGroup
                        tooltip: model.tooltip
                        checked: kubeViews.currentViewName == model.name
                    }
                }
            }
            Column {
                anchors {
                    bottom: parent.bottom
                    bottomMargin: Kube.Units.smallSpacing
                    horizontalCenter: parent.horizontalCenter
                }

                spacing: Kube.Units.largeSpacing - Kube.Units.smallSpacing
                Kube.Outbox {
                    height: Kube.Units.gridUnit * 1.5
                    width: height

                    Kube.ToolTip {
                        text: qsTr("Outbox")
                        visible: parent.hovered
                    }
                }

                Kube.IconButton {
                    id: logButton
                    iconName: Kube.Icons.info_inverted
                    onClicked: kubeViews.showView("log")
                    activeFocusOnTab: true
                    checkable: true
                    visible: false
                    Kube.Listener {
                        filter: Kube.Messages.errorPending
                        onMessageReceived: logButton.alert = message.errorPending
                    }
                    Kube.Listener {
                        filter: Kube.Messages.notificationPending
                        onMessageReceived: logButton.visible = true
                    }
                    checked: kubeViews.currentViewName == "log"
                    Controls2.ButtonGroup.group: viewButtonGroup
                    tooltip: qsTr("Notification View")
                }

                Kube.IconButton {
                    id: accountsButton
                    iconName: Kube.Icons.menu_inverted
                    onClicked: kubeViews.showView("accounts")
                    activeFocusOnTab: true
                    checkable: true
                    checked: kubeViews.currentViewName == "accounts"
                    Controls2.ButtonGroup.group: viewButtonGroup
                    tooltip: qsTr("Settings")
                }
            }
        }
        ViewManager {
            id: kubeViews
            anchors {
                top: mainContent.top
                bottom: mainContent.bottom
            }
            Layout.fillWidth: true

            extensionModel: extensionModel

            Component.onCompleted: {
                dontFocus = true
                prepareViewInBackground("log", {})
                showView("conversation")
                if (startupCheck.noAccount) {
                    showView("accounts")
                }
                dontFocus = false
            }

            Kube.Listener {
                filter: Kube.Messages.reply
                onMessageReceived: kubeViews.replaceView("composer", {message: message.mail, loadType: Kube.ComposerController.Reply, accountId: app.currentAccount})
            }

            Kube.Listener {
                filter: Kube.Messages.forward
                onMessageReceived: kubeViews.replaceView("composer", {message: message.mail, loadType: Kube.ComposerController.Forward, accountId: app.currentAccount})
            }

            Kube.Listener {
                filter: Kube.Messages.edit
                onMessageReceived: kubeViews.replaceView("composer", {message: message.mail, loadType: Kube.ComposerController.Draft, accountId: app.currentAccount})
            }

            Kube.Listener {
                filter: Kube.Messages.compose
                onMessageReceived: kubeViews.replaceView("composer", {newMessage: true, recipients: message.recipients, accountId: app.currentAccount})
            }

            Kube.Listener {
                filter: Kube.Messages.requestAccountsConfiguration
                onMessageReceived: kubeViews.showView("accounts")
            }

            Kube.Listener {
                filter: Kube.Messages.componentDone
                onMessageReceived: {
                    kubeViews.closeView()
                }
            }

            Kube.Listener {
                filter: Kube.Messages.requestLogin
                onMessageReceived: {
                    var view = loginView.createObject(kubeViews, {accountId: message.accountId})
                    view.forceActiveFocus()
                }
            }

            Component {
                id: loginView
                Kube.Popup {
                    id: popup
                    property alias accountId: login.accountId
                    visible: true
                    parent: Controls2.ApplicationWindow.overlay
                    height: app.height
                    width: app.width - app.sidebarWidth
                    x: app.sidebarWidth
                    y: 0
                    modal: true
                    closePolicy: Controls2.Popup.NoAutoClose
                    Kube.LoginAccount {
                        id: login
                        anchors {
                            fill: parent
                            bottomMargin: Kube.Units.largeSpacing
                        }
                        onDone: {
                            kubeViews.currentItem.forceActiveFocus()
                            popup.destroy()
                        }
                    }
                }
            }

        }
    }
    //END Main content

    //BEGIN Notification
    Kube.NotificationPopup {
        id: notificationPopup

        anchors {
            left: parent.left
            leftMargin: app.sidebarWidth - 3 // so it does not align with the border
            bottom: parent.bottom
            bottomMargin: Kube.Units.gridUnit * 4
        }
    }
    //END Notification
}
