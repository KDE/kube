/*
 *  Copyright (C) 2021 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import QtQuick.Controls 1.3 as Controls1
import QtQuick.Controls 2
import org.kube.framework 1.0 as Kube

Controls1.SplitView {
    id: root

    property bool pendingError: false;
    property bool pendingNotification: false;
    onPendingErrorChanged: {
        Kube.Fabric.postMessage(Kube.Messages.errorPending, {errorPending: pendingError})
    }
    onPendingNotificationChanged: {
        Kube.Fabric.postMessage(Kube.Messages.notificationPending, {notificationPending: pendingNotification})
    }

    StackView.onActivated: {
        root.pendingError = false;
        root.pendingNotification = false;
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
                //Ignore noise that we can't usefully render anyways
                if (!message.message) {
                    return
                }
                //Avoid highlighting the iconbutton again if we're already looking at this view.
                if (root.StackView.status != StackView.Active) {
                    if (message.type == Kube.Notifications.error) {
                        root.pendingError = true
                    }
                    root.pendingNotification = true
                }
                logModel.insert(message)
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

            model: Kube.InboundModel {
                id: logModel
                objectName: "inboundModel"
                onEntryAdded: {
                    Kube.Fabric.postMessage(Kube.Messages.displayNotification, message)
                }
            }

            onCurrentItemChanged: {
                if (!!currentItem.currentData.resource) {
                    details.resourceId = currentItem.currentData.resource
                }
                details.message = currentItem.currentData.message + "\n" + currentItem.currentData.details
                details.timestamp = currentItem.currentData.timestamp
                details.entities = currentItem.currentData.entities
                if (!!currentItem.currentData.subtype) {
                    details.subtype = currentItem.currentData.subtype
                } else {
                    details.subtype = ""
                }
            }

            delegate: Kube.MailListDelegate {
                id: delegateRoot
                height: Kube.Units.gridUnit * 5

                subject: model.data.subject
                unread: model.data.unread
                senderName: model.data.senderName
                date: model.data.date
                important: model.data.important
                trash: model.data.trash
                threadSize: model.data.threadSize
                mail: model.data.mail
            }
        }
    }
    Item {
        id: details
        property string subtype: ""
        property date timestamp
        property string message: ""
        property string resourceId: ""
        property var entities: []

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
            property string entityId: (details.entities && details.entities.length != 0) ? details.entities[0] : ""

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
                if (subtype == Kube.Notifications.transmissionError) {
                    return transmissionErrorComponent
                }
                if (subtype == Kube.Notifications.messageSent) {
                    return transmissionSuccessComponent
                }
                if (subtype == "mail") {
                    return conversationComponent
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
                        text: qsTr("%1: please check your credentials.").arg(accountName)
                        color: Kube.Colors.disabledTextColor
                        wrapMode: Text.Wrap
                    }
                }
                Kube.Button {
                    text: qsTr("Change Password")
                    onClicked: {
                        Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
                        Kube.Fabric.postMessage(Kube.Messages.requestLogin, {accountId: accountId})
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
                        text: qsTr("%1: please check your network connection and settings.").arg(accountName)
                        color: Kube.Colors.disabledTextColor
                        wrapMode: Text.Wrap
                    }
                }
                Kube.Button {
                    text: qsTr("Account Settings")
                    onClicked: {
                        Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
                        Kube.Fabric.postMessage(Kube.Messages.requestAccountsConfiguration, {})
                    }
                }
            }
        }
    }

    Component {
        id: transmissionErrorComponent
        Item {
            id: componentRoot
            Column {
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                spacing: Kube.Units.largeSpacing

                Kube.Heading {
                    id: heading
                    text: qsTr("Failed to send the message.")
                    color: Kube.Colors.warningColor
                }

                Column {
                    spacing: Kube.Units.largeSpacing

                    Repeater {
                        model: Kube.MailListModel {
                            filter: {
                                "entityId": componentRoot.parent ? componentRoot.parent.entityId : null,
                                "headersOnly": true,
                                "fetchMails": true
                            }
                        }
                        delegate: Column {
                            id: subHeadline

                            Kube.Label {
                                text: qsTr("Account: %1").arg(accountName)
                                color: Kube.Colors.disabledTextColor
                                wrapMode: Text.Wrap
                            }
                            Kube.Label {
                                text: qsTr("Subject: %1").arg(model.subject)
                                color: Kube.Colors.disabledTextColor
                                wrapMode: Text.Wrap
                            }
                            Kube.Label {
                                text: qsTr("To: %1").arg(model.to)
                                color: Kube.Colors.disabledTextColor
                                wrapMode: Text.Wrap
                            }
                            Kube.Label {
                                visible: !!model.cc
                                text: qsTr("Cc: %1").arg(model.cc)
                                color: Kube.Colors.disabledTextColor
                                wrapMode: Text.Wrap
                            }

                        }
                    }
                }

                Kube.Button {
                    text: qsTr("Try Again")
                    onClicked: {
                        Kube.Fabric.postMessage(Kube.Messages.sendOutbox, {})
                    }
                }
            }
        }
    }

    Component {
        id: transmissionSuccessComponent
        Item {
            id: componentRoot
            Column {
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                spacing: Kube.Units.largeSpacing

                Kube.Heading {
                    id: heading
                    text: qsTr("Succeeded to send the message.")
                }

                Column {
                    spacing: Kube.Units.largeSpacing

                    Repeater {
                        model: Kube.MailListModel {
                            filter: {
                                "entityId": componentRoot.parent ? componentRoot.parent.entityId : null,
                                "headersOnly": true,
                                "fetchMails": true
                            }
                        }
                        delegate: Column {
                            id: subHeadline

                            Kube.Label {
                                text: qsTr("Account: %1").arg(accountName)
                                color: Kube.Colors.disabledTextColor
                                wrapMode: Text.Wrap
                            }
                            Kube.Label {
                                text: qsTr("Subject: %1").arg(model.subject)
                                color: Kube.Colors.disabledTextColor
                                wrapMode: Text.Wrap
                            }
                            Kube.Label {
                                text: qsTr("To: %1").arg(model.to)
                                color: Kube.Colors.disabledTextColor
                                wrapMode: Text.Wrap
                            }
                            Kube.Label {
                                visible: !!model.cc
                                text: qsTr("Cc: %1").arg(model.cc)
                                color: Kube.Colors.disabledTextColor
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                }
            }
        }
    }
    Component {
        id: conversationComponent

                Kube.ConversationView {
                    id: componentRoot
                    objectName: "mailView"
                    activeFocusOnTab: true
                    model: Kube.MailListModel {
                        filter: {
                            "entityId": componentRoot.parent ? componentRoot.parent.entityId : null,
                            "headersOnly": false,
                            "fetchMails": true
                        }
                    }
                }
    }
}
