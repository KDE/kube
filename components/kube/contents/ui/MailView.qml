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
import QtQuick.Controls 1.3
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

SplitView {
    Rectangle {
        width: Kube.Units.gridUnit * 10
        Layout.minimumWidth: Kube.Units.gridUnit * 5

        color: Kube.Colors.textColor
        focus: true

        Kube.PositiveButton {
            id: newMailButton

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: Kube.Units.largeSpacing
            }

            text: qsTr("New Email")

            onClicked: {
                Kube.Fabric.postMessage(Kube.Messages.compose, {})
            }
        }

        Kube.InlineAccountSwitcher {
            id: accountFolderview

            activeFocusOnTab: true
            anchors {
                top: newMailButton.bottom
                topMargin: Kube.Units.largeSpacing
                bottom: statusBar.top
                left: newMailButton.left
                right: parent.right
            }
        }

        Item {
            id: statusBar
            anchors {
                topMargin: Kube.Units.smallSpacing
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            visible: false

            height: Kube.Units.gridUnit * 2

            Rectangle {
                id: border
                anchors {
                    right: parent.right
                    rightMargin: Kube.Units.smallSpacing
                    left: parent.left
                    leftMargin: Kube.Units.smallSpacing
                    bottomMargin: Kube.Units.smallSpacing
                    topMargin: Kube.Units.smallSpacing
                }
                height: 1
                color: Kube.Colors.viewBackgroundColor
                opacity: 0.3
            }

            Repeater {
                model: Kube.AccountsModel {
                    accountId: accountFolderview.currentAccount
                }

                Column {
                    anchors {
                        top: border.bottom
                        left: statusBar.left
                        right: statusBar.right
                        bottom: statusBar.bottom
                    }
                    spacing: Kube.Units.smallSpacing
                    Kube.Label {
                        id: statusText
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: false
                        color: Kube.Colors.highlightedTextColor
                        states: [
                            State {
                                name: "disconnected"; when: model.status == Kube.AccountsModel.OfflineStatus
                                PropertyChanges { target: statusBar; visible: true }
                                PropertyChanges { target: statusText; text: "Disconnected"; visible: true }
                            },
                            State {
                                name: "busy"; when: model.status == Kube.AccountsModel.BusyStatus
                                PropertyChanges { target: statusBar; visible: true }
                                PropertyChanges { target: statusText; text: "Synchronizing..."; visible: true }
                                PropertyChanges { target: progressBar; visible: true }
                            },
                            State {
                                name: "error"; when: model.status == Kube.AccountsModel.ErrorStatus
                                PropertyChanges { target: statusBar; visible: true }
                                //TODO get to an error description
                                PropertyChanges { target: statusText; text: "Error"; visible: true }
                            }
                        ]
                    }
                    Kube.ProgressBar {
                        id: progressBar
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: 2
                        width: parent.width - Kube.Units.smallSpacing * 2

                        indeterminate: true
                        visible: false

                        Kube.Listener {
                            filter: Kube.Messages.progressNotification
                            onMessageReceived: {
                                progressBar.indeterminate = false
                                progressBar.from = 0
                                progressBar.to = message.total
                                progressBar.value = message.progress
                            }
                        }
                    }
                }
            }
        }
    }

    Kube.MailListView  {
        id: mailListView
        width: Kube.Units.gridUnit * 20
        height: parent.height
        Layout.minimumWidth: Kube.Units.gridUnit * 10
    }

    Kube.ConversationView {
        id: mailView
        Layout.fillWidth: true
        Layout.minimumWidth: Kube.Units.gridUnit * 5
    }
}
