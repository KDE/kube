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
    Repeater {
        model: Kube.AccountsModel {
            accountId: root.accountId
        }

        Column {
            anchors.fill: root
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
