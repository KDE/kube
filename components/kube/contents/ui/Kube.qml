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
import QtQuick.Layouts 1.3
import QtQuick.Window 2.0

import QtQuick.Controls 2.0 as Controls2
import org.kube.framework 1.0 as Kube

Controls2.ApplicationWindow {
    id: app

    height: Screen.desktopAvailableHeight * 0.8
    width: Screen.desktopAvailableWidth * 0.8
    visible: true

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
        onMessageReceived: kubeViews.openComposer(true)
    }

    //BEGIN Shortcuts
    Shortcut {
        id: syncShortcut
        property variant folder: null
        sequence: StandardKey.Refresh
        enabled: !!folder
        onActivated: Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": folder})
    }
    Kube.Listener {
        filter: Kube.Messages.folderSelection
        onMessageReceived: {
            syncShortcut.folder = message.folder
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
            width: Kube.Units.gridUnit + Kube.Units.largeSpacing
            color: Kube.Colors.textColor

            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: Kube.Colors.viewBackgroundColor
                opacity: 0.3
            }

            Column {
                anchors {
                    top: parent.top
                    topMargin: Kube.Units.smallSpacing
                    horizontalCenter: parent.horizontalCenter
                }

                spacing: Kube.Units.largeSpacing - Kube.Units.smallSpacing

                Kube.IconButton {
                    iconName: Kube.Icons.search_inverted
                    onClicked: search.open()
                    activeFocusOnTab: true

                    Kube.ToolTip {
                        text: qsTr("search")
                        visible: parent.hovered
                    }
                }

                Kube.IconButton {
                    iconName: Kube.Icons.edit_inverted
                    onClicked: kubeViews.openComposer(false)
                    activeFocusOnTab: true

                    Kube.ToolTip {
                        text: qsTr("composer")
                        visible: parent.hovered
                    }
                }

                Kube.IconButton {
                    iconName: Kube.Icons.mail_inverted
                    onClicked: kubeViews.setMailView()
                    activeFocusOnTab: true

                    Kube.ToolTip {
                        text: qsTr("mails")
                        visible: parent.hovered
                    }
                }

                Kube.IconButton {
                    iconName: Kube.Icons.user_inverted
                    onClicked: kubeViews.setPeopleView()
                    activeFocusOnTab: true

                    Kube.ToolTip {
                        text: qsTr("people")
                        visible: parent.hovered
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
                        text: qsTr("outbox")
                        visible: parent.hovered
                    }
                }

                Kube.IconButton {
                    iconName: Kube.Icons.error_inverted
                    onClicked: kubeViews.setLogView()

                    Kube.ToolTip {
                        text: qsTr("logview")
                        visible: parent.hovered
                    }
                }

                Kube.IconButton {
                    iconName: Kube.Icons.menu_inverted
                    onClicked: kubeViews.setAccountsView()

                    Kube.ToolTip {
                        text: qsTr("settings")
                        visible: parent.hovered
                    }
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
            initialItem: mailView

            Kube.Listener {
                filter: Kube.Messages.componentDone
                onMessageReceived: kubeViews.pop(Controls2.StackView.Immediate)
            }

            //TODO replacing here while a composer is open is destructive
            function setPeopleView() {
                if (currentItem != peopleView) {
                    kubeViews.replace(null, peopleView, Controls2.StackView.Immediate)
                }
            }
            function setMailView() {
                if (currentItem != mailView) {
                    kubeViews.replace(null, mailView, Controls2.StackView.Immediate)
                }
            }
            function setAccountsView() {
                kubeViews.push(accountsView, {}, Controls2.StackView.Immediate)
            }
            function setLogView() {
                if (currentItem != logView) {
                    kubeViews.replace(null, logView, Controls2.StackView.Immediate)
                }
            }

            function openComposer(newMessage) {
                kubeViews.push(composerView, {newMessage: newMessage}, Controls2.StackView.Immediate)
            }
            function openComposerWithMail(mail, openAsDraft) {
                kubeViews.push(composerView, {message: mail, loadAsDraft: openAsDraft}, Controls2.StackView.Immediate)
            }

            onCurrentItemChanged: {
                //TODO with qt 5.8 use Controls2.StackView.onActivated
                if (currentItem == peopleView) {
                    Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "contacts"})
                }
            }

            //These items are not visible until pushed onto the stack, so we keep them in resources instead of items
            resources: [
                //Not components so we maintain state
                MailView {
                    id: mailView
                    anchors.fill: parent
                },
                PeopleView {
                    id: peopleView
                    anchors.fill: parent
                },
                //Not a component because otherwise we can't log stuff
                LogView {
                    id: logView
                    anchors.fill: parent
                }
            ]
            //A component so it's always destroyed when we're done
            Component {
                id: composerView
                ComposerView {
                }
            }
            Component {
                id: accountsView
                AccountsView {
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

    //BEGIN Search
    Kube.Popup {
        id: search

        width: app.width * 0.6
        height: Kube.Units.gridUnit * 3

        x: app.width * 0.2
        y: app.height * 0.2

        RowLayout {
            anchors.centerIn: parent
            width: parent.width

            Kube.TextField {
                id: searchField
                focus: true
                Layout.fillWidth: true
                placeholderText: qsTr("Filter...     (only applies to the mail list for now)")
                Keys.onReturnPressed: {
                    Kube.Fabric.postMessage(Kube.Messages.search, {"filterString": searchField.text})
                    search.close()
                }
            }

            Kube.Button {
                text: qsTr("Go")

                onClicked: {
                    Kube.Fabric.postMessage(Kube.Messages.search, {"filterString": searchField.text})
                    search.close()
                }
            }
        }
    }
    //END Search
}
