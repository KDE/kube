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


import QtQuick 2.9
import QtQuick.Controls 1.3 as Controls1
import QtQuick.Controls 2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.View {
    id: root
    property string currentAccount: Kube.Context.currentAccountId
    property variant currentFolder: null

    property date currentDate: new Date()
    property bool autoUpdateDate: true

    //We have to hardcode because all the mapToItem/mapFromItem functions are garbage
    searchArea: Qt.rect(ApplicationWindow.window.sidebarWidth + todoView.parent.x, 0, todoView.parent.width + todoDetails.parent.width, height)

    onRefresh: {
        if (!!root.currentFolder) {
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": root.currentFolder});
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": root.currentAccount, "type": "folder"})
        } else {
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": root.currentAccount})
        }
    }

    states: [
        State {
            name: "doing"
            PropertyChanges {target: root; currentFolder: null}
            StateChangeScript {script: accountSwitcher.clearSelection()}
        },
        State {
            name: "all"
            PropertyChanges {target: root; currentFolder: null}
            StateChangeScript {script: accountSwitcher.clearSelection()}
        },
        State {
            name: "calendar"
            PropertyChanges {target: root; currentFolder: accountSwitcher.currentCalendar}
        }
    ]
    state: "doing"

    Kube.Listener {
        filter: Kube.Messages.search
        onMessageReceived: root.triggerSearch()
    }

    helpViewComponent: Kube.HelpPopup {
        ListModel {
            ListElement { description: qsTr("Go to top of list:"); shortcut: "t" }
            ListElement { description: qsTr("Go to bottom of list:"); shortcut: "b" }
            ListElement { description: qsTr("Go to next todo:"); shortcut: "j" }
            ListElement { description: qsTr("Go to previous todo:"); shortcut: "k" }
            ListElement { description: qsTr("Create new todo:"); shortcut: "c" }
            ListElement { description: qsTr("Edit todo:"); shortcut: "e" }
            ListElement { description: qsTr("Show this help text:"); shortcut: "?" }
        }
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['t']
        onActivated: todoView.currentIndex = 0
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['b']
        onActivated: todoView.currentIndex = todoView.count - 1
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['j']
        onActivated: todoView.incrementCurrentIndex()
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['k']
        onActivated: todoView.decrementCurrentIndex()
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['c']
        onActivated: editorPopup.createObject(root, {}).open()
    }
    Shortcut {
        enabled: root.isCurrentView
        sequences: ['e']
        onActivated: todoDetails.edit()
    }
    Shortcut {
        enabled: root.isCurrentView
        sequence: "?"
        onActivated: root.showHelp()
    }

    ButtonGroup { id: viewButtonGroup }

    Timer {
        running: autoUpdateDate
        interval: 2000; repeat: true
        onTriggered: root.currentDate = new Date()
    }

    Controls1.SplitView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Rectangle {
            width: Kube.Units.gridUnit * 10
            Layout.fillHeight: parent.height
            color: Kube.Colors.darkBackgroundColor

            Column {
                id: topLayout
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }

                Kube.PositiveButton {
                    id: newMailButton
                    objectName: "newMailButton"

                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    focus: true
                    text: qsTr("New Todo")
                    onClicked: editorPopup.createObject(root, {}).open()
                }

                Item {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    height: Kube.Units.gridUnit
                }

                Kube.TextButton {
                    id: doingViewButton
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: qsTr("Doing")
                    textColor: Kube.Colors.highlightedTextColor
                    checkable: true
                    checked: root.state == "doing"
                    horizontalAlignment: Text.AlignHLeft
                    ButtonGroup.group: viewButtonGroup
                    onClicked: root.state = "doing"
                }
                Kube.TextButton {
                    id: allViewButton
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: qsTr("All")
                    textColor: Kube.Colors.highlightedTextColor
                    checkable: true
                    checked: root.state == "all"
                    horizontalAlignment: Text.AlignHLeft
                    ButtonGroup.group: viewButtonGroup
                    onClicked: root.state = "all"
                }
            }

            Kube.CalendarSelector {
                id: accountSwitcher
                activeFocusOnTab: true
                selectionEnabled: true
                anchors {
                    top: topLayout.bottom
                    topMargin: Kube.Units.largeSpacing
                    bottom: statusBarContainer.top
                    left: topLayout.left
                    right: parent.right
                    rightMargin: Kube.Units.largeSpacing
                }
                contentType: "todo"
                onCurrentCalendarChanged: {
                    if (currentCalendar) {
                        root.state = "calendar"
                    }
                }
            }

            Item {
                id: statusBarContainer
                anchors {
                    topMargin: Kube.Units.smallSpacing
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                height: childrenRect.height

                Rectangle {
                    id: border
                    visible: statusBar.visible
                    anchors {
                        right: parent.right
                        left: parent.left
                        margins: Kube.Units.smallSpacing
                    }
                    height: 1
                    color: Kube.Colors.viewBackgroundColor
                    opacity: 0.3
                }
                Kube.StatusBar {
                    id: statusBar
                    accountId: root.currentAccount
                    height: Kube.Units.gridUnit * 2
                    anchors {
                        top: border.bottom
                        left: statusBarContainer.left
                        right: statusBarContainer.right
                    }
                }
            }
        }

        Rectangle {
            width: Kube.Units.gridUnit * 18
            Layout.fillHeight: parent.height

            color: "transparent"
            border.width: 1
            border.color: Kube.Colors.buttonColor

            Kube.ListView {
                id: todoView
                anchors.fill: parent
                Layout.minimumWidth: Kube.Units.gridUnit * 10

                onCurrentItemChanged: {
                    if (currentItem) {
                        var currentData = currentItem.currentData;
                        todoDetails.controller = controllerComponent.createObject(parent, {"todo": currentData.domainObject})
                    }
                }

                Column {
                    anchors.centerIn: parent
                    visible: todoView.count === 0
                    Kube.Label {
                        text: qsTr("Nothing here yet...")
                    }
                    Kube.PositiveButton {
                        visible: doingViewButton.checked
                        text: qsTr("Pick some tasks")
                        onClicked: {
                            allViewButton.checked = true
                            allViewButton.clicked()
                        }
                    }
                    Kube.PositiveButton {
                        visible: allViewButton.checked
                        text: qsTr("Add a new task")
                        onClicked: editorPopup.createObject(root, {}).open()
                    }
                }

                model: Kube.TodoModel {
                    id: todoModel
                    filter: {
                        "account": accountSwitcher.currentAccount,
                        "calendars": root.currentFolder ? [root.currentFolder] : accountSwitcher.enabledCalendars,
                        "doing": root.state == "doing",
                        "string": root.filter
                    }
                }
                delegate: Kube.ListDelegate {
                    id: delegateRoot
                    property bool buttonsVisible: delegateRoot.hovered

                    width: todoView.availableWidth
                    height: Kube.Units.gridUnit * 3

                    color: Kube.Colors.viewBackgroundColor
                    border.color: Kube.Colors.backgroundColor
                    border.width: 1


                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        drag.target: parent
                        drag.filterChildren: true
                        onReleased: {
                            if (parent.Drag.drop() == Qt.MoveAction) {
                                Kube.Fabric.postMessage(Kube.Messages.moveToCalendar, {"todo": model.domainObject, "calendarId": parent.Drag.target.calendarId})
                            }
                        }
                        onClicked: delegateRoot.clicked()
                    }

                    states: [
                        State {
                            name: "dnd"
                            when: mouseArea.drag.active

                            PropertyChanges {target: mouseArea; cursorShape: Qt.ClosedHandCursor}
                            PropertyChanges {target: delegateRoot; opacity: 0.5}
                            ParentChange {target: delegateRoot; parent: root; x: x; y: y}
                        }
                    ]

                    Drag.active: mouseArea.drag.active
                    Drag.hotSpot.x: mouseArea.mouseX
                    Drag.hotSpot.y: mouseArea.mouseY
                    Drag.source: delegateRoot

                    Item {
                        id: content

                        anchors {
                            fill: parent
                            margins: Kube.Units.smallSpacing
                        }

                        Rectangle {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                            }
                            width: Kube.Units.smallSpacing
                            height: width
                            radius: width / 2

                            color: model.color
                        }
                        Column {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                leftMargin:  Kube.Units.largeSpacing
                            }

                            Kube.Label{
                                width: content.width - Kube.Units.gridUnit * 3
                                text: model.summary
                                color: delegateRoot.textColor
                                font.strikeout: model.complete
                                font.bold: model.doing && root.state != "doing"
                                maximumLineCount: 2
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                            }
                            Kube.Label {
                                visible: delegateRoot.hovered
                                text: model.calendar
                                color: delegateRoot.disabledTextColor
                                font.italic: true
                                width: delegateRoot.width - Kube.Units.gridUnit * 3
                                elide: Text.ElideRight
                            }
                        }

                        Kube.Label {
                            id: date
                            anchors {
                                right: parent.right
                                bottom: parent.bottom
                            }

                            function sameDay(date1, date2) {
                                return date1.getFullYear() == date2.getFullYear() && date1.getMonth() == date2.getMonth() && date1.getDate() == date2.getDate()
                            }

                            function daysSince(date1, date2) {
                                //FIXME this is not going to work at month borders
                                return (date1.getDate() - date2.getDate())
                            }

                            //TODO deal with start dates
                            function formatDueDateTime(date) {
                                const today = root.currentDate
                                if (sameDay(date, today)) {
                                    return qsTr("Due today")
                                }
                                const nextWeekToday = today.getTime() + ((24*60*60*1000) * 7);
                                if (date.getTime() < nextWeekToday && date.getTime() > today.getTime()) {
                                    return Qt.formatDateTime(date, "dddd") + qsTr(" (%1 days)").arg(daysSince(date, today))
                                }
                                if (date.getTime() < today.getTime()) {
                                    return qsTr("Overdue for %1 days").arg(daysSince(today, date))
                                }
                                return Qt.formatDateTime(date, "dd MMM yyyy")
                            }

                            visible: !isNaN(model.date) && !delegateRoot.buttonsVisible
                            text: (!isNaN(model.dueDate) && !model.complete) ? formatDueDateTime(model.dueDate) : Qt.formatDateTime(model.date, "dd MMM yyyy")
                            font.italic: true
                            color: delegateRoot.disabledTextColor
                            font.pointSize: Kube.Units.tinyFontSize
                        }
                    }

                    Kube.Icon {
                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                            margins: Kube.Units.smallSpacing
                        }

                        visible:  model.important && !delegateRoot.buttonsVisible
                        iconName: Kube.Icons.isImportant
                    }

                    Column {
                        id: buttons

                        anchors {
                            right: parent.right
                            margins: Kube.Units.smallSpacing
                            verticalCenter: parent.verticalCenter
                        }

                        visible: delegateRoot.buttonsVisible
                        opacity: 0.7

                        Kube.IconButton {
                            iconName: model.doing ? Kube.Icons.listRemove : Kube.Icons.addNew
                            activeFocusOnTab: false
                            tooltip: model.doing ? qsTr("Unpick") : qsTr("Pick")
                            onClicked: {
                                var controller = controllerComponent.createObject(parent, {"todo": model.domainObject});
                                if (controller.complete) {
                                    controller.complete = false
                                }
                                controller.doing = !controller.doing;
                                controller.saveAction.execute();
                            }
                        }

                        Kube.IconButton {
                            iconName: Kube.Icons.checkbox
                            checked: model.complete
                            activeFocusOnTab: false
                            tooltip: qsTr("Done!")
                            onClicked: {
                                var controller = controllerComponent.createObject(parent, {"todo": model.domainObject});
                                controller.complete = !controller.complete;
                                controller.saveAction.execute();
                            }
                        }

                    }
                }
            }
        }
        Rectangle {
            Layout.fillHeight: parent.height
            Layout.fillWidth: true

            color: Kube.Colors.paperWhite

            TodoView {
                id: todoDetails
                anchors.fill: parent
                // onDone: popup.close()
            }
        }
        Component {
            id: controllerComponent
            Kube.TodoController {
            }
        }

    }

    Component {
        id: editorPopup
        Kube.Popup {
            id: popup

            x: root.width * 0.15
            y: root.height * 0.15

            width: root.width * 0.7
            height: root.height * 0.7
            padding: 0
            closePolicy: Popup.NoAutoClose
            TodoEditor {
                id: editor
                focus: true
                anchors.fill: parent
                accountId: root.currentAccount
                currentFolder: root.currentFolder
                doing: doingViewButton.checked
                onDone: popup.close()
            }
        }
    }

}
