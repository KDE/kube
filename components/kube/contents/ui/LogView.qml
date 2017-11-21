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
        width: parent.width/3
        Layout.fillHeight: true

        Kube.Listener {
            filter: Kube.Messages.notification
            onMessageReceived: {
                if (message.type == Kube.Notifications.error) {
                    root.pendingError = true
                }
                var error = {timestamp: new Date(), message: message.message, details: message.details, resource: message.resource}
                if (logModel.count > 0) {
                    var lastEntry = logModel.get(0)
                    //Merge if we get an entry of the same subtype
                    if (lastEntry.subtype && lastEntry.subtype == message.subtype) {
                        logModel.set(0, {type: message.type, subtype: message.subtype, errors: [error].concat(lastEntry.errors)})
                        return
                    }
                }
                logModel.insert(0, {type: message.type, subtype: message.subtype, errors: [error]})
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
                fill: parent
            }

            clip: true

            model: ListModel {
                id: logModel
                objectName: "logModel"
            }

            onCurrentItemChanged: {
                var error = currentItem.currentData.errors.get(0)
                if (!!error.resource) {
                    details.resourceId = error.resource
                }
                details.message = error.message + "\n" + error.details
                details.timestamp = error.timestamp
                if (!!currentItem.currentData.subtype) {
                    details.subtype = currentItem.currentData.subtype
                } else {
                    details.subtype = ""
                }
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
                        rightMargin: Kube.Units.smallSpacing
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
        property string subtype: ""
        property date timestamp
        property string message: ""
        property string resourceId: ""

        Kube.ModelIndexRetriever {
            id: retriever
            model: Kube.AccountsModel {
                resourceId: details.resourceId
            }
        }

        Loader {
            id: detailsLoader
            visible: message != ""
            clip: true
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
            }
            property date timestamp: details.timestamp
            property string message: details.message
            property string resourceId: details.resourceId
            property string accountId: retriever.currentData ? retriever.currentData.accountId : ""
            property string accountName: retriever.currentData ? retriever.currentData.name : ""

            function getComponent(subtype) {
                if (subtype == Kube.Notifications.loginError) {
                    return loginErrorComponent
                }
                if (subtype == Kube.Notifications.hostNotFoundError) {
                    return hostNotFoundErrorComponent
                }
                if (subtype == Kube.Notifications.connectionError) {
                    return hostNotFoundErrorComponent
                }
                return detailsComponent
            }

            sourceComponent: getComponent(details.subtype)
        }
    }

    Component {
        id: detailsComponent
        Rectangle {
            color: Kube.Colors.viewBackgroundColor
            GridLayout {
                id: gridLayout
                Layout.minimumWidth: 0
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                columns: 2
                Kube.Label {
                    text: qsTr("Account:")
                    visible: accountName
                }
                Kube.Label {
                    Layout.fillWidth: true
                    text: accountName
                    visible: accountName
                    elide: Text.ElideRight
                }
                Kube.Label {
                    text: qsTr("Account Id:")
                    visible: accountId
                }
                Kube.Label {
                    text: accountId
                    visible: accountId
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Kube.Label {
                    text: qsTr("Resource Id:")
                    visible: resourceId
                }
                Kube.Label {
                    text: resourceId
                    visible: resourceId
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Kube.Label {
                    text: qsTr("Timestamp:")
                }
                Kube.Label {
                    text: Qt.formatDateTime(timestamp, " hh:mm:ss dd MMM yyyy")
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Kube.Label {
                    text: qsTr("Message:")
                    Layout.alignment: Qt.AlignTop
                }
                Kube.Label {
                    text: message
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
                Item {
                    Layout.columnSpan: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }

            Kube.SelectableItem {
                layout: gridLayout
            }
        }
    }

    Component {
        id: loginErrorComponent
        Item {
            Column {
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                spacing: Kube.Units.largeSpacing
                Column {
                    Kube.Heading {
                        id: heading
                        text: qsTr("Failed to login")
                        color: Kube.Colors.warningColor
                    }

                    Kube.Label {
                        id: subHeadline
                        text: accountName + ": " + qsTr("Please check your credentials.")
                        color: Kube.Colors.disabledTextColor
                        wrapMode: Text.Wrap
                    }
                }
                Kube.Button {
                    text: qsTr("Change Password")
                    onClicked: {
                        Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
                        Kube.Fabric.postMessage(Kube.Messages.requestLogin, {})
                    }
                }
            }
        }
    }

    Component {
        id: hostNotFoundErrorComponent
        Item {
            Column {
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                spacing: Kube.Units.largeSpacing
                Column {
                    Kube.Heading {
                        id: heading
                        text: qsTr("Host not found")
                        color: Kube.Colors.warningColor
                    }

                    Kube.Label {
                        id: subHeadline
                        text: accountName + ": " + qsTr("Please check your network connection and settings.")
                        color: Kube.Colors.disabledTextColor
                        wrapMode: Text.Wrap
                    }
                }
                Kube.Button {
                    text: qsTr("Account settings")
                    onClicked: {
                        Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
                        Kube.Fabric.postMessage(Kube.Messages.requestAccountsConfiguration, {})
                    }
                }
            }
        }
    }
}
