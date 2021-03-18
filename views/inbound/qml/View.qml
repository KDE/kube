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

Kube.View {
    id: root

    property date currentDate: new Date()
    property bool autoUpdateDate: true

    property bool pendingError: false;
    property bool pendingNotification: false;
    onPendingErrorChanged: {
        Kube.Fabric.postMessage(Kube.Messages.errorPending, {errorPending: pendingError})
    }
    onPendingNotificationChanged: {
        Kube.Fabric.postMessage(Kube.Messages.notificationPending, {notificationPending: pendingNotification})
    }

    onRefresh: {
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {})
        inboundModel.refresh()
    }

    Timer {
        running: autoUpdateDate
        interval: 2000; repeat: true
        onTriggered: root.currentDate = new Date()
    }


    Controls1.SplitView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Item {
            id: accountList
            width: parent.width/3
            Layout.fillHeight: true


            Kube.Label {
                anchors.centerIn: parent
                visible: listView.count == 0
                text: qsTr("Nothing here...")
            }


            Component {
                id: sectionHeading
                Rectangle {
                    width: ListView.view.width
                    height: childrenRect.height

                    Kube.Heading {
                        text: section == "event" ? "Coming up" : "Recently"
                    }
                }
            }

            Kube.ListView {
                id: listView
                anchors {
                    fill: parent
                }

                clip: true

                section.property: "type"
                section.criteria: ViewSection.FullString
                section.delegate: sectionHeading

                model: Kube.InboundModel {
                    id: inboundModel
                    objectName: "inboundModel"
                    onEntryAdded: {
                        Kube.Fabric.postMessage(Kube.Messages.displayNotification, message)
                    }
                    currentDate: root.currentDate
                }

                onCurrentItemChanged: {
                    if (!currentItem || !currentItem.currentData) {
                        return
                    }
                    var currentData = currentItem.currentData;
                    if (!!currentData.resource) {
                        details.resourceId = currentData.resource
                    }
                    details.message = currentData.message + "\n" + currentItem.currentData.details
                    details.timestamp = currentData.timestamp
                    details.entities = currentData.entities
                    details.itemData = currentData.data
                    if (!!currentData.subtype) {
                        details.subtype = currentData.subtype
                    } else {
                        details.subtype = ""
                    }
                    if (currentData.data.mail && currentData.data.unread) {
                        Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": currentData.data.mail})
                    }
                }

                delegate: Kube.GenericListDelegate {
                    id: delegateRoot

                    property var isMail: model.type == "mail"
                    property var domainObject: model.data.domainObject
                    property var important: model.data.important
                    height: isMail ? Kube.Units.gridUnit * 5 : (Kube.Units.gridUnit * 3 + 2 * Kube.Units.smallSpacing)

                    onDropped: {
                        if (dropAction == Qt.MoveAction) {
                            if (isMail) {
                                delegateRoot.visible = false
                            } else {
                                Kube.Fabric.postMessage(Kube.Messages.moveToCalendar, {"event": delegateRoot.domainObject, "calendarId": dropTarget.calendarId})
                            }
                        }
                    }
                    function sameDay(date1, date2) {
                        return date1.getFullYear() == date2.getFullYear() && date1.getMonth() == date2.getMonth() && date1.getDate() == date2.getDate()
                    }

                    function daysSince(date1, date2) {
                        //FIXME this is not going to work at month borders
                        return (date1.getDate() - date2.getDate())
                    }

                    function formatStartDateTime(date) {
                        //TODO remove reference to root
                        const today = currentDate
                        if (sameDay(date, today)) {
                            return qsTr("Today, ")+ Qt.formatDateTime(date, "hh:mm")
                        }

                        const daysTo = daysSince(date, today)
                        if (daysTo == 1) {
                            return qsTr("Tomorrow, ") + Qt.formatDateTime(date, "hh:mm")
                        }
                        if (daysTo <= 7) {
                            return Qt.formatDateTime(date, "dddd") + qsTr(" (%1 days)").arg(daysSince(date, today))
                        }
                        if (date.getTime() < today.getTime()) {
                            return qsTr("%1 days ago").arg(daysSince(today, date))
                        }
                        return Qt.formatDateTime(date, "dd MMM yyyy")
                    }

                    function formatDateTime(date) {
                        const today = new Date()
                        if (sameDay(date, today)) {
                            return Qt.formatDateTime(date, "hh:mm")
                        }
                        const lastWeekToday = today.getTime() - ((24*60*60*1000) * 7);
                        if (date.getTime() >= lastWeekToday) {
                            return Qt.formatDateTime(date, "ddd hh:mm")
                        }
                        return Qt.formatDateTime(date, "dd MMM yyyy")
                    }

                    mainText: model.data.subject
                    subText: isMail ? model.data.senderName : model.data.calendar
                    dateText: isMail ? formatDateTime(model.data.date) : formatStartDateTime(model.data.date)
                    active: model.data.unread
                    disabled: model.data.complete
                    strikeout: model.data.complete ? model.data.complete : false
                    counter: isMail ? model.data.threadSize : 0
                    subtextVisible: true
                    subtextDisabled: false

                    Component {
                        id: importantStatusComponent
                        Kube.Icon {
                            iconName: Kube.Icons.isImportant
                            visible:  delegateRoot.important
                        }
                    }

                    statusDelegate: isMail ? importantStatusComponent : null

                    Component {
                        id: mailButtonComponent
                        Column {
                            Kube.IconButton {
                                id: restoreButton
                                iconName: Kube.Icons.undo
                                visible: !!delegateRoot.trash
                                onClicked: Kube.Fabric.postMessage(Kube.Messages.restoreFromTrash, {"mail": delegateRoot.domainObject})
                                activeFocusOnTab: false
                                tooltip: qsTr("Restore from trash")
                            }

                            Kube.IconButton {
                                id: readButton
                                iconName: Kube.Icons.markAsRead
                                visible: model.data.unread && !model.data.trash
                                onClicked: {
                                    Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": delegateRoot.domainObject})
                                }
                                tooltip: qsTr("Mark as read")
                            }

                            Kube.IconButton {
                                id: unreadButton
                                iconName: Kube.Icons.markAsUnread
                                visible: !model.data.unread && !model.data.trash
                                onClicked: Kube.Fabric.postMessage(Kube.Messages.markAsUnread, {"mail": delegateRoot.domainObject})
                                activeFocusOnTab: false
                                tooltip: qsTr("Mark as unread")
                            }

                            Kube.IconButton {
                                id: importantButton
                                iconName: delegateRoot.important ? Kube.Icons.markImportant : Kube.Icons.markUnimportant
                                visible: !!delegateRoot.domainObject
                                onClicked: Kube.Fabric.postMessage(Kube.Messages.setImportant, {"mail": delegateRoot.domainObject, "important": !model.data.important})
                                activeFocusOnTab: false
                                tooltip: qsTr("Mark as important")
                            }

                            Kube.IconButton {
                                id: deleteButton
                                objectName: "deleteButton"
                                iconName: Kube.Icons.moveToTrash
                                visible: !!delegateRoot.domainObject
                                onClicked: Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": delegateRoot.domainObject})
                                activeFocusOnTab: false
                                tooltip: qsTr("Move to trash")
                            }
                        }
                    }

                    Component {
                        id: eventButtonComponent
                        Column {
                            //Cancel
                            //Reschedule
                            //Ignore?
                            Kube.IconButton {
                                iconName: Kube.Icons.checkbox
                                activeFocusOnTab: false
                                tooltip: qsTr("Done!")
                            }
                        }
                    }

                    buttonDelegate: isMail ? mailButtonComponent : eventButtonComponent

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
            property var itemData: null

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
                property var itemData: details.itemData

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

                    if (subtype == "event") {
                        return eventComponent
                    }
                    return detailsComponent
                }

                sourceComponent: getComponent(details.subtype)
            }
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
                    "mail": componentRoot.parent ? componentRoot.parent.itemData.mail : null,
                    "headersOnly": false,
                    "fetchMails": true
                }
            }
        }
    }


    Component {
        id: controllerComponent
        Kube.EventController {
        }
    }
    Component {
        id: eventComponent
        EventView {
            id: componentRoot
            property var occurrence: componentRoot.parent ? componentRoot.parent.itemData.occurrence : null
            onOccurrenceChanged: {
                //Workaround because we need to create a new controller to reload the occurrence
                componentRoot.controller = controllerComponent.createObject(parent, {"eventOccurrence": occurrence})
            }
        }
    }
}
