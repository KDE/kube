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

            height: Kube.Units.gridUnit * 2

            Repeater {
                model: Kube.AccountsModel {
                    accountId: accountFolderview.currentAccount
                }

                Column {
                    anchors.fill: statusBar
                    spacing: Kube.Units.smallSpacing
                    Kube.Label {
                        id: statusText
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: false
                        color: Kube.Colors.highlightedTextColor
                        states: [
                            State {
                                name: "disconnected"; when: model.status == Kube.AccountsModel.OfflineStatus
                                PropertyChanges { target: statusText; text: "Offline"; visible: true }
                            },
                            State {
                                name: "busy"; when: model.status == Kube.AccountsModel.BusyStatus
                                PropertyChanges { target: statusText; text: "Busy"; visible: true }
                                PropertyChanges { target: progressBar; visible: true }
                            },
                            State {
                                name: "error"; when: model.status == Kube.AccountsModel.ErrorStatus
                                PropertyChanges { target: statusText; text: "Error"; visible: true }
                            }
                        ]
                    }
                    Controls2.ProgressBar {
                        id: progressBar
                        indeterminate: true
                        visible: false
                        height: Kube.Units.smallSpacing
                        width: parent.width

                        background: Rectangle {
                            color: Kube.Colors.backgroundColor
                            radius: 3
                        }

                        contentItem: Item {
                            Rectangle {
                                width: progressBar.visualPosition * parent.width
                                height: parent.height
                                radius: 2
                                color: Kube.Colors.highlightColor
                            }
                        }


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
