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
    id: root

    property bool pendingError: false;

    Controls2.StackView.onActivated: {
        root.pendingError = false;
        //Always select the latest notification
        listView.currentIndex = 0
    }

    Item {
        id: accountList
        width: parent.width/2
        Layout.fillHeight: true

        Kube.Listener {
            filter: Kube.Messages.notification
            onMessageReceived: {
                if (message.type == Kube.Notifications.error) {
                    root.pendingError = true
                }
                var error = {timestamp: new Date(), message: message.message, details: message.details, resource: message.resource}
                logModel.insert(0, {type: message.type, errors: [error]})
            }
        }

        Kube.Label {
            anchors.centerIn: parent
            visible: listView.count == 0
            text: qsTr("Nothing here...")
        }

        Kube.ListView {
            id: listView
            anchors {
                margins: Kube.Units.largeSpacing
                fill: parent
            }

            clip: true

            model: ListModel {
                id: logModel
                objectName: "logModel"
            }

            onCurrentItemChanged: {
                if (!!currentItem.currentData.resource) {
                    details.resourceId = currentItem.currentData.errors.get(0).resource
                }
                details.message = currentItem.currentData.message + "\n" + currentItem.currentData.errors.get(0).details
                details.timestamp = currentItem.currentData.errors.get(0).timestamp
            }
            delegate: Kube.ListDelegate {
                border.color: Kube.Colors.buttonColor
                border.width: 1
                Kube.Label {
                    id: description
                    anchors {
                        top: parent.top
                        topMargin: Kube.Units.smallSpacing
                        left: parent.left
                        leftMargin: Kube.Units.largeSpacing
                    }
                    height: Kube.Units.gridUnit
                    width: parent.width - Kube.Units.largeSpacing * 2
                    text: model.type == Kube.Notifications.error ? qsTr("Error") : qsTr("Info")
                }

                Kube.Label {
                    id: message
                    anchors {
                        topMargin: Kube.Units.smallSpacing
                        top: description.bottom
                        left: parent.left
                        leftMargin: Kube.Units.largeSpacing
                    }
                    height: Kube.Units.gridUnit
                    width: parent.width - Kube.Units.largeSpacing * 2
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    color: Kube.Colors.disabledTextColor
                    text: model.errors.get(0).message
                }

                Kube.Label {
                    id: date

                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                    }
                    text: Qt.formatDateTime(model.errors.get(0).timestamp, " hh:mm:ss dd MMM yyyy")
                    font.italic: true
                    color: Kube.Colors.disabledTextColor
                    font.pointSize: Kube.Units.smallFontSize
                }
            }
        }
    }
    Item {
        id: details
        property date timestamp
        property string message: ""
        property string resourceId: ""
        property string accountId: retriever.currentData ? retriever.currentData.accountId : ""
        property string accountName: retriever.currentData ? retriever.currentData.name : ""

        Kube.ModelIndexRetriever {
            id: retriever
            model: Kube.AccountsModel {
                resourceId: details.resourceId
            }
        }

        Rectangle {
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
            }
            visible: details.message != ""
            clip: true
            color: Kube.Colors.viewBackgroundColor
            GridLayout {
                id: gridLayout
                Layout.minimumWidth: 0
                anchors {
                    top: parent.top
                    left: parent.left
                }
                width: parent.width
                columns: 2
                Kube.Label {
                    text: qsTr("Account:")
                    visible: details.accountName
                }
                Kube.Label {
                    Layout.fillWidth: true
                    text: details.accountName
                    visible: details.accountName
                    elide: Text.ElideRight
                }
                Kube.Label {
                    text: qsTr("Account Id:")
                    visible: details.accountId
                }
                Kube.Label {
                    text: details.accountId
                    visible: details.accountId
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Kube.Label {
                    text: qsTr("Resource Id:")
                    visible: details.resourceId
                }
                Kube.Label {
                    text: details.resourceId
                    visible: details.resourceId
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Kube.Label {
                    text: qsTr("Timestamp:")
                }
                Kube.Label {
                    text: Qt.formatDateTime(details.timestamp, " hh:mm:ss dd MMM yyyy")
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Kube.Label {
                    text: qsTr("Message:")
                    Layout.alignment: Qt.AlignTop
                }
                Kube.Label {
                    text: details.message
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
                Item {
                    Layout.columnSpan: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
                //TODO offer a possible explanation for known errors and a path to resolution.
            }

            Kube.SelectableItem {
                layout: gridLayout
            }
        }
    }
}
