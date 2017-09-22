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
import QtQuick.Layouts 1.3
import QtQuick.Window 2.0

import QtQuick.Controls 2.0 as Controls2
import org.kube.framework 1.0 as Kube

Controls2.ApplicationWindow {
    id: app

    property int sidebarWidth: Kube.Units.gridUnit + Kube.Units.largeSpacing
    property bool small: app.width < Kube.Units.gridUnit * 50

    height: Screen.desktopAvailableHeight * 0.8
    width: Screen.desktopAvailableWidth * 0.8
    visible: true

    //Application default font
    font.family: Kube.Font.fontFamily

    //Application context
    property variant currentFolder
    onCurrentFolderChanged: {
        if (!!currentFolder) {
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": currentFolder})
        }
    }
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
        running: !!app.currentFolder
        repeat: true
        onTriggered: Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": app.currentFolder})
    }

    Kube.StartupCheck {
        id: startupCheck
    }

    //Listener
    Kube.Listener {
        filter: Kube.Messages.accountSelection
        onMessageReceived: app.currentAccount = message.account
    }

    Kube.Listener {
        filter: Kube.Messages.folderSelection
        onMessageReceived: app.currentFolder = message.folder
    }

    Kube.Listener {
        filter: Kube.Messages.notification
        onMessageReceived: {
            notificationPopup.notify(message.message);
        }
    }

    Kube.Listener {
        filter: Kube.Messages.reply
        onMessageReceived: {
            kubeViews.openComposerWithMail(message.mail, false)
        }
    }

    Kube.Listener {
        filter: Kube.Messages.edit
        onMessageReceived: {
            kubeViews.openComposerWithMail(message.mail, true)
        }
    }

    Kube.Listener {
        filter: Kube.Messages.compose
        onMessageReceived: kubeViews.openComposer(true, message.recipients)
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
        onActivated: {
            Kube.Fabric.postMessage(Kube.Messages.unlockKeyring, {accountId: app.currentAccount})
        }
        sequence: "Ctrl+l"
    }
    Shortcut {
        id: syncShortcut
        sequence: StandardKey.Refresh
        onActivated: !!app.currentFolder ? Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": app.currentFolder}) : Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": app.currentAccount})
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
            color: Kube.Colors.textColor

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

                Kube.IconButton {
                    id: composerButton
                    iconName: Kube.Icons.edit_inverted
                    onClicked: kubeViews.openComposer(false, [])
                    activeFocusOnTab: true
                    checkable: true
                    Controls2.ButtonGroup.group: viewButtonGroup
                    tooltip: qsTr("composer")
                }

                Kube.IconButton {
                    id: mailButton
                    iconName: Kube.Icons.mail_inverted
                    onClicked: kubeViews.setMailView()
                    activeFocusOnTab: true
                    checkable: true
                    checked: true
                    Controls2.ButtonGroup.group: viewButtonGroup
                    tooltip: qsTr("mails")
                }

                Kube.IconButton {
                    id: peopleButton
                    iconName: Kube.Icons.user_inverted
                    onClicked: kubeViews.setPeopleView()
                    activeFocusOnTab: true
                    checkable: true
                    Controls2.ButtonGroup.group: viewButtonGroup
                    tooltip: qsTr("people")
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
                        text: qsTr("outbox")
                        visible: parent.hovered
                    }
                }

                Kube.IconButton {
                    id: logButton
                    iconName: Kube.Icons.error_inverted
                    onClicked: kubeViews.setLogView()
                    activeFocusOnTab: true
                    checkable: true
                    Controls2.ButtonGroup.group: viewButtonGroup
                    tooltip: qsTr("logview")
                }

                Kube.IconButton {
                    id: accountsButton
                    iconName: Kube.Icons.menu_inverted
                    onClicked: kubeViews.setAccountsView()
                    activeFocusOnTab: true
                    checkable: true
                    Controls2.ButtonGroup.group: viewButtonGroup
                    tooltip: qsTr("settings")
                }
            }
        }
        Controls2.StackView {
            id: kubeViews

            anchors {
                top: mainContent.top
                bottom: mainContent.bottom
            }
            Layout.fillWidth: true

            Kube.Listener {
                filter: Kube.Messages.componentDone
                onMessageReceived: {
                    kubeViews.pop(Controls2.StackView.Immediate)
                    if (!!app.currentAccount && !Kube.Keyring.isUnlocked(app.currentAccount)) {
                        kubeViews.setLoginView()
                    }
                }
            }

            ///Replace the current view (we can't go back to the old view, and we destroy the old view)
            function replaceView(view) {
                if (currentItem != view) {
                    kubeViews.replace(null, view, {}, Controls2.StackView.Immediate)
                }
            }

            ///Push a new view on the stack (the old view remains, and we can go back once done)
            function pushView(view, properties) {
                kubeViews.push(view, properties, Controls2.StackView.Immediate)
            }

            //TODO replacing here while a composer is open is destructive
            function setPeopleView() {
                replaceView(peopleView)
            }

            function setMailView() {
                replaceView(mailView)
            }

            function setAccountsView() {
                pushView(accountsView, {})
            }

            function setLogView() {
                replaceView(logView)
            }

            function setLoginView() {
                if (currentItem != loginView) {
                    pushView(loginView, {accountId: currentAccount})
                }
            }

            function openComposer(newMessage, recipients) {
                pushView(composerView, {newMessage: newMessage, recipients: recipients})
            }

            function openComposerWithMail(mail, openAsDraft) {
                pushView(composerView, {message: mail, loadAsDraft: openAsDraft})
            }

            onCurrentItemChanged: {
                if (currentItem) {
                    currentItem.forceActiveFocus()
                }
            }

            Component.onCompleted: {
                //Setup the initial item stack
                if (!currentItem) {
                    setMailView();
                    if (startupCheck.noAccount) {
                        setAccountsView()
                    } else {
                        if (!!app.currentAccount && !Kube.Keyring.isUnlocked(app.currentAccount)) {
                            setLoginView()
                        }
                    }
                }
            }

            //These items are not visible until pushed onto the stack, so we keep them in resources instead of items
            resources: [
                //Not components so we maintain state
                MailView {
                    id: mailView
                    anchors.fill: parent
                    Controls2.StackView.onActivated: mailButton.checked = true
                    Controls2.StackView.onDeactivated: mailButton.checked = false
                },
                PeopleView {
                    id: peopleView
                    anchors.fill: parent
                    Controls2.StackView.onActivated: peopleButton.checked = true
                    Controls2.StackView.onDeactivated: peopleButton.checked = false
                },
                //Not a component because otherwise we can't log stuff
                LogView {
                    id: logView
                    anchors.fill: parent
                    Controls2.StackView.onActivated: logButton.checked = true
                    Controls2.StackView.onDeactivated: logButton.checked = false
                }
            ]
            //A component so it's always destroyed when we're done
            Component {
                id: composerView
                ComposerView {
                    anchors.fill: parent
                    Controls2.StackView.onActivated: composerButton.checked = true
                    Controls2.StackView.onDeactivated: composerButton.checked = false
                }
            }
            Component {
                id: accountsView
                AccountsView {
                    anchors.fill: parent
                    Controls2.StackView.onActivated: accountsButton.checked = true
                    Controls2.StackView.onDeactivated: accountsButton.checked = false
                }
            }
            Component {
                id: loginView
                LoginView {
                    anchors.fill: parent
                }
            }
        }
    }
    //END Main content

    //BEGIN Notification
    Kube.NotificationPopup {
        id: notificationPopup

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
    }
    //END Notification
}
