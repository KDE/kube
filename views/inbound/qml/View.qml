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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.3 as Controls1
import QtQuick.Controls 2
import org.kube.framework 1.0 as Kube

import "dateutils.js" as DateUtils

Kube.View {
    id: root

    property date currentDate: new Date()
    property bool autoUpdateDate: true

    property bool pendingError: false
    property bool pendingNotification: false

    property var modelFilter: {
        "inbound": true,
        "recent": false,
        "important": false,
        "folder": null,
        "string": root.filter,
        "headersOnly": true,
    }

    property var currentFolder: null

    onPendingErrorChanged: {
        Kube.Fabric.postMessage(Kube.Messages.errorPending, {errorPending: pendingError})
    }

    onPendingNotificationChanged: {
        Kube.Fabric.postMessage(Kube.Messages.notificationPending, {notificationPending: pendingNotification})
    }

    //We have to hardcode because all the mapToItem/mapFromItem functions are garbage
    searchArea: Qt.rect(ApplicationWindow.window.sidebarWidth + listItem.x, 0, (details.x + details.width) - listItem.x, (details.y + details.height) - listItem.y)

    onFilterChanged: {
        if (filter) {
            inboundModel.enableNotifications = false;
        } else {
            inboundModel.enableNotifications = true;
        }
        root.modelFilter = {
            "inbound": root.modelFilter.inbound,
            "recent": root.modelFilter.recent,
            "important": root.modelFilter.important,
            "folder": root.modelFilter.folder,
            "string": filter,
            "headersOnly": true
        }
        Kube.Fabric.postMessage(Kube.Messages.searchString, {"searchString": filter})
    }

    Kube.Listener {
        filter: Kube.Messages.folderSelection
        onMessageReceived: {
            root.currentFolder = message.folder
            accountSwitcher.selectEntity(message.folder)
        }
    }

    onActivated: {
        //In case of an initial load we may have positioned before the view is actually expanded,
        //so position again once the view is activated.
        listView.positionViewAtIndex(listView.currentIndex, ListView.Center);
    }

    onRefresh: {
        if (!root.modelFilter.inbound && !!root.currentFolder) {
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": root.currentFolder});
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": Kube.Context.currentAccountId, "type": "folder"})
        } else {
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {})
        }
    }

    onCurrentFolderChanged: {
        if (!!root.currentFolder) {
            root.modelFilter = {
                "inbound": false,
                "recent": false,
                "important": false,
                "folder": accountSwitcher.currentEntity,
                "string": root.filter,
            }
        }
    }

    Timer {
        running: autoUpdateDate
        interval: 2000; repeat: true
        onTriggered: root.currentDate = new Date()
    }

    Kube.Listener {
        filter: Kube.Messages.search
        onMessageReceived: root.triggerSearch()
    }

    helpViewComponent: Kube.HelpPopup {
        ListModel {
            ListElement { description: qsTr("Jump to top of threadlist:"); shortcut: "t" }
            ListElement { description: qsTr("Jump to next thread:"); shortcut: "j" }
            ListElement { description: qsTr("Jump to previous thread:"); shortcut: "k" }
            ListElement { description: qsTr("Jump to next message:"); shortcut: "n" }
            ListElement { description: qsTr("Jump to previous message:"); shortcut: "p" }
            ListElement { description: qsTr("Jump to next folder:"); shortcut: "f,n" }
            ListElement { description: qsTr("Jump to previous folder:"); shortcut: "f,p" }
            ListElement { description: qsTr("Compose new message:"); shortcut: "c" }
            ListElement { description: qsTr("Reply to the currently focused message:"); shortcut: "r" }
            ListElement { description: qsTr("Delete the currently focused message:"); shortcut: "d" }
            ListElement { description: qsTr("Mark the currently focused message as important:"); shortcut: "i" }
            ListElement { description: qsTr("Mark the currently focused message as unread:"); shortcut: "u" }
            ListElement { description: qsTr("Show this help text:"); shortcut: "?" }
        }
    }

    Shortcut {
        enabled: root.isCurrentView
        sequences: ['j']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectNextConversation, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['k']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectPreviousConversation, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['t']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectTopConversation, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['Shift+J']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.scrollConversationDown, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['Shift+K']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.scrollConversationUp, {})
    }
    Shortcut {
        sequences: ['n']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectNextMessage, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['p']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectPreviousMessage, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['f,n']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectNextFolder, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['f,p']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.selectPreviousFolder, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['c']
        onActivated: Kube.Fabric.postMessage(Kube.Messages.compose, {})
    }
    Shortcut {
        enabled: root.isCurrentView
        sequence: "?"
        onActivated: root.showHelp()
    }

    Controls1.SplitView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Kube.LeftSidebar {
            Layout.fillHeight: parent.height
            buttons: [
                Kube.PositiveButton {
                    id: newMailButton
                    objectName: "newMailButton"
                    Layout.fillWidth: true
                    focus: true
                    text: qsTr("New Email")
                    onClicked: Kube.Fabric.postMessage(Kube.Messages.compose, {})
                },
                ColumnLayout {
                    Kube.TextButton {
                        Layout.fillWidth: true
                        text: qsTr("Inbound")
                        textColor: Kube.Colors.highlightedTextColor
                        checkable: true
                        checked: root.modelFilter.inbound
                        horizontalAlignment: Text.AlignHLeft
                        onClicked: {
                            root.modelFilter = {
                                "inbound": true,
                                "recent": false,
                                "important": false,
                                "folder": null,
                                "string": root.filter,
                                "headersOnly": true,
                            }
                            accountSwitcher.clearSelection()
                        }
                        Kube.IconButton {
                            anchors {
                                right: parent.right
                                verticalCenter: parent.verticalCenter
                            }
                            visible: parent.hovered
                            padding: 0
                            iconName: Kube.Icons.overflowMenu_inverted
                            onClicked: {
                                configComponent.createObject(root).open()
                            }
                            activeFocusOnTab: false
                            tooltip: qsTr("Configure")
                        }
                    }
                    Kube.TextButton {
                        Layout.fillWidth: true
                        text: qsTr("Recent")
                        textColor: Kube.Colors.highlightedTextColor
                        checkable: true
                        checked: root.modelFilter.recent
                        horizontalAlignment: Text.AlignHLeft
                        onClicked: {
                            root.modelFilter = {
                                "inbound": false,
                                "recent": true,
                                "important": false,
                                "folder": null,
                                "string": root.filter,
                                "headersOnly": true,
                            }
                            accountSwitcher.clearSelection()
                        }
                    }
                    Kube.TextButton {
                        Layout.fillWidth: true
                        text: qsTr("Important")
                        textColor: Kube.Colors.highlightedTextColor
                        checkable: true
                        checked: root.modelFilter.important
                        horizontalAlignment: Text.AlignHLeft
                        onClicked: {
                            root.modelFilter = {
                                "inbound": false,
                                "recent": false,
                                "important": true,
                                "folder": null,
                                "string": root.filter,
                                "headersOnly": true,
                            }
                            accountSwitcher.clearSelection()
                        }
                    }
                }
            ]


            Kube.EntitySelector {
                id: accountSwitcher
                Layout.fillWidth: true
                Layout.fillHeight: true
                activeFocusOnTab: true
                selectionEnabled: true
                entityType: "folder"
                roles: ["name", "enabled"]
                sortRole: "customMail"
                Kube.EntityController {
                    id: entityController
                }
                onEntityCreated: {
                    entityController.create({type: "folder", account: accountId, entity: {
                        "name": text,
                        "enabled": true,
                    }})
                }
                onEntityRemoved: {
                    entityController.remove(entity)
                }
                onCurrentEntityChanged: {
                    Kube.Fabric.postMessage(Kube.Messages.folderSelection, {
                        "folder": currentEntity,
                        "trash": false
                    })
                }
            }
        }

        StackLayout {
            width: parent.width/3
            Layout.fillHeight: true

            Item {
                id: listItem

                function reselect() {
                    console.warn("Reselect")
                    var idx = listView.currentIndex
                    listView.currentIndex = -1;
                    listView.currentIndex = idx;
                }

                Kube.Listener {
                    filter: Kube.Messages.selectTopConversation
                    onMessageReceived: {
                        listView.currentIndex = 0
                        listView.forceActiveFocus()
                    }
                }

                Kube.Listener {
                    filter: Kube.Messages.selectNextConversation
                    onMessageReceived: {
                        listView.incrementCurrentIndex()
                        listView.forceActiveFocus()
                    }
                }

                Kube.Listener {
                    filter: Kube.Messages.selectPreviousConversation
                    onMessageReceived: {
                        listView.decrementCurrentIndex()
                        listView.forceActiveFocus()
                    }
                }

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
                    objectName: "listView"
                    anchors.fill: parent

                    clip: true
                    focus: true
                    reuseItems: true

                    property double startTime: 0

                    onActiveFocusChanged: {
                        if (activeFocus && currentIndex < 0) {
                            currentIndex = 0
                        }
                    }

                    section {
                        property: "type"
                        criteria: ViewSection.FullString
                        delegate: root.modelFilter.inbound ? sectionHeading : null
                    }

                    Keys.onPressed: {
                        //Not implemented as a shortcut because we want it only to apply if we have the focus
                        if (currentItem.currentData.data.mail) {
                            var currentMail = currentItem.currentData.data.mail;
                            if (event.text == "d" || event.key == Qt.Key_Delete) {
                                Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": currentMail})
                            } else if (event.text == "r") {
                                Kube.Fabric.postMessage(Kube.Messages.reply, {"mail": currentMail})
                            } else if (event.text == "i") {
                                Kube.Fabric.postMessage(Kube.Messages.setImportant, {"mail": currentMail, "important": !currentItem.currentData.data.important})
                            } else if (event.text == "u") {
                                Kube.Fabric.postMessage(Kube.Messages.markAsUnread, {"mail": currentMail})
                            }
                        }
                        if (event.key == Qt.Key_Home) {
                            listView.currentIndex = 0
                        }
                    }

                    model: Kube.InboundModel {
                        id: inboundModel
                        objectName: "inboundModel"
                        property var enableNotifications: false
                        onEntryAdded: {
                            if (enableNotifications) {
                                Kube.Fabric.postMessage(Kube.Messages.displayNotification, message)
                            }
                        }

                        filter: root.modelFilter

                        onFilterChanged: {
                            enableNotifications = false;
                            //Avoid selecting each item at the current index until the initial items are loaded
                            listView.currentIndex = -1;
                            listView.startTime = new Date().getTime()
                        }

                        onFetchingMore: {
                            enableNotifications = false;
                        }

                        onInitialItemsLoaded: {
                            enableNotifications = true;
                            //Because this is also emitted on fetchMore
                            if (listView.currentIndex == -1) {
                                //Make sure the view is up-to-date before positioning
                                listView.forceLayout()
                                listView.currentIndex = inboundModel.firstRecentIndex();
                                listView.positionViewAtIndex(listView.currentIndex, ListView.Center);
                                console.info("Initial items loaded: " + (new Date().getTime() - listView.startTime) + " ms")
                            }
                        }
                        currentDate: root.currentDate
                    }

                    onCurrentItemChanged: {
                        if (!currentItem || !currentItem.currentData) {
                            details.subtype = ""
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

                        property bool isMail: model.type == "mail"
                        property var domainObject: model.data.domainObject
                        property bool important: isMail && model.data.important
                        height: Kube.Units.gridUnit * 3 + 2 * Kube.Units.smallSpacing

                        onDropped: {
                            if (dropAction == Qt.MoveAction) {
                                if (isMail) {
                                    delegateRoot.visible = false
                                } else {
                                    Kube.Fabric.postMessage(Kube.Messages.moveToCalendar, {"event": delegateRoot.domainObject, "calendarId": dropTarget.targetId})
                                }
                            }
                        }

                        function formatStartDateTime(date, today) {
                            if (DateUtils.sameDay(date, today)) {
                                return qsTr("Today, ")+ Qt.formatDateTime(date, "hh:mm")
                            }

                            const daysTo = DateUtils.daysSince(date, today)
                            if (daysTo == 1) {
                                return qsTr("Tomorrow, ") + Qt.formatDateTime(date, "hh:mm")
                            }
                            if (daysTo <= 7) {
                                return Qt.formatDateTime(date, "dddd") + qsTr(" (in %1 days)").arg(daysTo)
                            }
                            if (date.getTime() < today.getTime()) {
                                return qsTr("%1 days ago").arg(DateUtils.daysSince(today, date))
                            }
                            return Qt.formatDateTime(date, "dd MMM yyyy")
                        }

                        function formatDateTime(date) {
                            const today = new Date()
                            if (DateUtils.sameDay(date, today)) {
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
                        dateText: isMail ? formatDateTime(model.data.date) : formatStartDateTime(model.data.date, root.currentDate)
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
                            Row {
                                Column {
                                    Kube.IconButton {
                                        id: ignoreButton
                                        visible: root.modelFilter.inbound
                                        padding: 0
                                        iconName: Kube.Icons.listRemove
                                        onClicked: inboundModel.ignoreSender(delegateRoot.domainObject)
                                        activeFocusOnTab: false
                                        tooltip: qsTr("Ignore sender")
                                    }
                                }
                                Column {
                                    Kube.IconButton {
                                        id: restoreButton
                                        iconName: Kube.Icons.undo
                                        visible: !!delegateRoot.trash
                                        padding: 0
                                        onClicked: Kube.Fabric.postMessage(Kube.Messages.restoreFromTrash, {"mail": delegateRoot.domainObject})
                                        activeFocusOnTab: false
                                        tooltip: qsTr("Restore from trash")
                                    }

                                    Kube.IconButton {
                                        id: readButton
                                        iconName: Kube.Icons.markAsRead
                                        visible: model.data.unread && !model.data.trash
                                        padding: 0
                                        onClicked: {
                                            Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": delegateRoot.domainObject})
                                        }
                                        tooltip: qsTr("Mark as read")
                                    }

                                    Kube.IconButton {
                                        id: unreadButton
                                        iconName: Kube.Icons.markAsUnread
                                        visible: !model.data.unread && !model.data.trash
                                        padding: 0
                                        onClicked: Kube.Fabric.postMessage(Kube.Messages.markAsUnread, {"mail": delegateRoot.domainObject})
                                        activeFocusOnTab: false
                                        tooltip: qsTr("Mark as unread")
                                    }

                                    Kube.IconButton {
                                        id: importantButton
                                        iconName: delegateRoot.important ? Kube.Icons.markImportant : Kube.Icons.markUnimportant
                                        visible: !!delegateRoot.domainObject
                                        padding: 0
                                        onClicked: Kube.Fabric.postMessage(Kube.Messages.setImportant, {"mail": delegateRoot.domainObject, "important": !model.data.important})
                                        activeFocusOnTab: false
                                        tooltip: qsTr("Mark as important")
                                    }

                                    Kube.IconButton {
                                        id: deleteButton
                                        objectName: "deleteButton"
                                        iconName: Kube.Icons.moveToTrash
                                        visible: !!delegateRoot.domainObject
                                        padding: 0
                                        onClicked: Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": delegateRoot.domainObject})
                                        activeFocusOnTab: false
                                        tooltip: qsTr("Move to trash")
                                    }
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
                                    padding: 0
                                }
                            }
                        }

                        buttonDelegate: isMail ? mailButtonComponent : eventButtonComponent

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

            function mailFilter() {
                return {
                    "mail": componentRoot.parent ? componentRoot.parent.itemData.mail : null,
                    "headersOnly": false,
                    "fetchMails": true,
                    //TODO hide trash in non-trash folders
                    //Don't hide trash in the trash folder
                    // "hideTrash": root.hideTrash,
                    // "hideNonTrash": root.hideNonTrash
                }
            }

            objectName: "mailView"
            activeFocusOnTab: true
            model: Kube.MailListModel {
                id: mailViewModel
                filter: mailFilter()
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

    Component {
        id: configComponent
        Kube.ScrollablePopup {
            id: configPopup

            onAccepted: {
                inboundModel.config = {
                    "folderNameBlacklist": folderNameBlacklist.text.split(', '),
                    "folderSpecialPurposeBlacklist": folderSpecialPurposeBlacklist.text.split(', '),
                    "perFolderMimeMessageWhitelistFilter": JSON.parse(perFolderMimeMessageWhitelistFilter.text),
                    "senderBlacklist": senderBlacklist.text.split(', '),
                    "toBlacklist": toBlacklist.text.split(', '),
                    "senderNameContainsFilter": senderNameContainsFilter.text,
                };
                configPopup.close()
            }

            GridLayout {
                id: grid
                width: parent.width
                height: childrenRect.height
                columns: 2
                columnSpacing: Kube.Units.largeSpacing
                rowSpacing: Kube.Units.largeSpacing

                Kube.Label {
                    text: qsTr("Folder name blacklist")
                    Layout.alignment: Qt.AlignRight
                }
                Kube.TextField {
                    id: folderNameBlacklist
                    focus: true
                    Layout.fillWidth: true
                    placeholderText: qsTr("Comma separated folder names")
                    text: inboundModel.config.folderNameBlacklist.join(', ')
                }

                Kube.Label {
                    text: qsTr("Per Folder Mime Message Whitelist")
                    Layout.alignment: Qt.AlignRight
                }
                Kube.TextField {
                    id: perFolderMimeMessageWhitelistFilter
                    focus: true
                    Layout.fillWidth: true
                    placeholderText: JSON.stringify({"foldername1": "filterstring1", "foldername2": "filterstring2",})
                    text: JSON.stringify(inboundModel.config.perFolderMimeMessageWhitelistFilter)
                }

                Kube.Label {
                    text: qsTr("Folder special purpose blacklist")
                    Layout.alignment: Qt.AlignRight
                }
                Kube.TextField {
                    id: folderSpecialPurposeBlacklist
                    focus: true
                    Layout.fillWidth: true
                    placeholderText: qsTr("Comma separated special purpose types. Example: drafts, trash, sent")
                    text: inboundModel.config.folderSpecialPurposeBlacklist.join(', ')
                }

                Kube.Label {
                    text: qsTr("Sender blacklist")
                    Layout.alignment: Qt.AlignRight
                }
                Kube.TextField {
                    id: senderBlacklist
                    focus: true
                    Layout.fillWidth: true
                    placeholderText: qsTr("Comma separated sender email addresses")
                    text: inboundModel.config.senderBlacklist.join(', ')
                }

                Kube.Label {
                    text: qsTr("To blacklist")
                    Layout.alignment: Qt.AlignRight
                }
                Kube.TextField {
                    id: toBlacklist
                    focus: true
                    Layout.fillWidth: true
                    placeholderText: qsTr("Comma separated sender email addresses")
                    text: inboundModel.config.toBlacklist.join(', ')
                }

                Kube.Label {
                    text: qsTr("Sender name contains filter")
                    Layout.alignment: Qt.AlignRight
                }
                Kube.TextField {
                    id: senderNameContainsFilter
                    focus: true
                    Layout.fillWidth: true
                    placeholderText: qsTr("Comma separated sender email addresses")
                    text: inboundModel.config.senderNameContainsFilter
                }
            }
        }
    }
}
