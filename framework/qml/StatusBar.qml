/*
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
import org.kube.framework 1.0 as Kube

Item {
    id: root
    property string accountId: ""
    property string currentFolderName: entityLoader.name ? entityLoader.name : ""
    property string currentFolderId: ""
    property string errorText: ""
    visible: false

    Kube.EntityLoader {
        id: entityLoader
        type: "folder"
        roles: ["name"]
        accountId: root.accountId
        entityId: root.currentFolderId
        property variant name
    }

    Repeater {
        model: Kube.AccountsModel {
            accountId: root.accountId
        }

        Item {
            anchors.fill: root
            anchors.leftMargin: Kube.Units.smallSpacing
            anchors.rightMargin: Kube.Units.smallSpacing
            Column {
                anchors.fill: parent
                Kube.Label {
                    id: statusText
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: Kube.Colors.highlightedTextColor
                    states: [
                        State {
                            name: "disconnected"; when: model.status == Kube.AccountsModel.OfflineStatus
                            PropertyChanges { target: root; visible: true }
                            PropertyChanges { target: statusText; text: qsTr("Disconnected"); visible: true }
                            PropertyChanges { target: descriptionText; visible: root.errorText != "" }
                        },
                        State {
                            name: "busy"; when: model.status == Kube.AccountsModel.BusyStatus
                            PropertyChanges { target: root; visible: true }
                            PropertyChanges { target: statusText; text: root.currentFolderName.length > 0 ? "Synchronizing " + root.currentFolderName: "Synchronizing..."; visible: true }
                            PropertyChanges { target: progressBar; visible: true }
                        },
                        State {
                            name: "error"; when: model.status == Kube.AccountsModel.ErrorStatus
                            PropertyChanges { target: root; visible: true }
                            PropertyChanges { target: statusText; text: root.errorText; visible: true }
                            PropertyChanges { target: descriptionText; visible: true }
                        }
                    ]
                }
                Kube.Label {
                    id: descriptionText
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    visible: false
                    color: Kube.Colors.disabledTextColor
                    text: root.errorText
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }
                Kube.ProgressBar {
                    id: progressBar
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        topMargin: Kube.Units.smallSpacing
                    }
                    height: 2
                    width: parent.width

                    indeterminate: true
                    visible: false

                    Kube.Listener {
                        filter: Kube.Messages.progressNotification
                        onMessageReceived: {
                            progressBar.indeterminate = false
                            progressBar.from = 0
                            progressBar.to = message.total
                            progressBar.value = message.progress
                            if (message.folderId) {
                                root.currentFolderId = message.folderId
                            } else {
                                root.currentFolderId = ""
                            }
                        }
                    }
                    Kube.Listener {
                        filter: Kube.Messages.errorNotification
                        onMessageReceived: {
                            root.errorText = message.message
                        }
                    }
                }
            }
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
            Kube.IconButton {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                visible: progressBar.visible && (hovered || mouseArea.containsMouse)
                iconName: Kube.Icons.remove
                onClicked: Kube.Fabric.postMessage(Kube.Messages.abortSynchronization, {})
                tooltip: qsTr("Abort")
            }
        }
    }
}
