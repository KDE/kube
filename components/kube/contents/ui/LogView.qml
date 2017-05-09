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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3 as Controls
import QtQuick.Controls 2.0 as Controls2
import org.kube.framework 1.0 as Kube

Controls.SplitView {
    Item {
        id: accountList
        width: parent.width/2
        Layout.fillHeight: true

        Kube.Listener {
            filter: Kube.Messages.notification
            onMessageReceived: {
                logModel.insert(0, {message: message.message, timestamp: new Date(), resource: message.resource});
            }
        }

        Item {
            id: statusBar
            anchors {
                topMargin: Kube.Units.smallSpacing
                top: parent.top
                left: parent.left
                right: parent.right
            }

            height: Kube.Units.gridUnit * 2

            Repeater {
                model: Kube.AccountsModel {
                    id: accountsModel
                }

                Column {
                    anchors.fill: statusBar
                    spacing: Kube.Units.smallSpacing
                    Row {
                        Kube.Label {
                            color: Kube.Colors.textColor
                            text: "Account: " + model.name
                        }
                        Kube.Label {
                            id: statusText
                            color: Kube.Colors.textColor
                            visible: false
                            states: [
                                State {
                                    name: "disconnected"; when: accountsModel.status == Kube.AccountsModel.OfflineStatus
                                    PropertyChanges { target: statusText; text: "Offline"; visible: true }
                                },
                                State {
                                    name: "busy"; when: accountsModel.status == Kube.AccountsModel.BusyStatus
                                    PropertyChanges { target: statusText; text: "Busy"; visible: true }
                                    PropertyChanges { target: progressBar; visible: true }
                                }
                            ]
                        }
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

        ListView {
            id: listView

            anchors {
                top: statusBar.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                topMargin: Kube.Units.largeSpacing
            }

            clip: true

            model: ListModel {
                id: logModel
            }

            onCurrentItemChanged: {
                details.resourceId = currentItem.currentData.resource
                details.message = currentItem.currentData.message
                details.timestamp = currentItem.currentData.timestamp
            }

            delegate: Rectangle {
                property variant currentData: model
                height: Kube.Units.gridUnit * 3
                width: listView.width

                border.color: Kube.Colors.buttonColor
                border.width: 1
                color: listView.currentIndex == index ? Kube.Colors.highlightColor : Kube.Colors.viewBackgroundColor

                Kube.Label {
                    id: resource
                    anchors {
                        top: parent.top
                        topMargin: Kube.Units.smallSpacing
                        left: parent.left
                        leftMargin: Kube.Units.largeSpacing
                    }
                    height: Kube.Units.gridUnit
                    width: parent.width - Kube.Units.largeSpacing * 2
                    text: "Resource: " + model.resource
                }

                Kube.Label {
                    id: message
                    anchors {
                        topMargin: Kube.Units.smallSpacing
                        top: resource.bottom
                        left: parent.left
                        leftMargin: Kube.Units.largeSpacing
                    }
                    height: Kube.Units.gridUnit
                    width: parent.width - Kube.Units.largeSpacing * 2
                    maximumLineCount: 1
                    elide: Text.ElideRight

                    text: model.message
                }

                Kube.Label {
                    id: date

                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                    }
                    text: Qt.formatDateTime(model.timestamp, " hh:mm:ss dd MMM yyyy")
                    font.italic: true
                    color: Kube.Colors.disabledTextColor
                    font.pointSize: 9
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onClicked: {
                        listView.currentIndex = index
                    }
                }
            }
        }
    }
    Rectangle {
        id: details
        property date timestamp
        property string message
        property variant resourceId
        GridLayout {
            anchors.fill: parent
            columns: 2
            Kube.Label {
                text: "Resource:"
            }
            Kube.Label {
                text: details.resourceId
            }
            Kube.Label {
                text: "Timestamp:"
            }
            Kube.Label {
                text: Qt.formatDateTime(details.timestamp, " hh:mm:ss dd MMM yyyy")
            }
            Kube.Label {
                text: "Message:"
            }
            Kube.Label {
                text: details.message
            }
            Item {
                Layout.columnSpan: 2
                Layout.fillHeight: true
            }
        }
    }
}
