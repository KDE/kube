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
    property alias currentAccount: accountSwitcher.currentAccount
    // property variant currentFolder: null

    //We have to hardcode because all the mapToItem/mapFromItem functions are garbage
    // searchArea: Qt.rect(ApplicationWindow.window.sidebarWidth + mailListView.parent.x, 0, (mailView.x + mailView.width) - mailListView.parent.x, (mailView.y + mailView.height) - mailListView.y)

    onFilterChanged: {
        Kube.Fabric.postMessage(Kube.Messages.searchString, {"searchString": filter})
    }

    onRefresh: {
        // if (!!root.currentFolder) {
        //     Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": root.currentFolder});
        //     Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": root.currentAccount, "type": "folder"})
        // } else {
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": root.currentAccount})
        // }
    }

    Kube.Listener {
        filter: Kube.Messages.search
        onMessageReceived: root.triggerSearch()
    }

    helpViewComponent: Kube.HelpPopup {
        ListModel {
            ListElement { description: qsTr("Go to next todo:"); shortcut: "j" }
            ListElement { description: qsTr("Go to previous todo:"); shortcut: "k" }
            ListElement { description: qsTr("Create new todo:"); shortcut: "c" }
            ListElement { description: qsTr("Show this help text:"); shortcut: "?" }
        }
    }

    Shortcut {
        sequences: ['j']
        onActivated: todoView.incrementCurrentIndex()
    }
    Shortcut {
        sequences: ['k']
        onActivated: todoView.decrementCurrentIndex()
    }
    Shortcut {
        sequences: ['c']
        onActivated: editorPopup.createObject(root, {}).open()
    }
    Shortcut {
        enabled: root.isCurrentView
        sequence: "?"
        onActivated: root.showHelp()
    }

    ButtonGroup { id: viewButtonGroup }

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
                    checked: true
                    horizontalAlignment: Text.AlignHLeft
                    ButtonGroup.group: viewButtonGroup
                    onClicked: todoModel.filter = {"doing": true}
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
                    horizontalAlignment: Text.AlignHLeft
                    ButtonGroup.group: viewButtonGroup
                    onClicked: todoModel.filter = {}
                }
            }

            Kube.CalendarSelector {
                id: accountSwitcher
                activeFocusOnTab: true
                anchors {
                    top: topLayout.bottom
                    topMargin: Kube.Units.largeSpacing
                    bottom: statusBarContainer.top
                    left: topLayout.left
                    right: parent.right
                    rightMargin: Kube.Units.largeSpacing
                }
                contentType: "todo"
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
                    calendarFilter: accountSwitcher.enabledCalendars
                    filter: {"doing": true}
                }
                delegate: Kube.ListDelegate {
                    id: delegateRoot
                    //Required for D&D
                    // property var mail: model.mail
                    property bool buttonsVisible: delegateRoot.hovered

                    width: todoView.availableWidth
                    height: Kube.Units.gridUnit * 3

                    color: Kube.Colors.viewBackgroundColor
                    border.color: Kube.Colors.backgroundColor
                    border.width: 1

                    Item {
                        id: content

                        anchors {
                            fill: parent
                            margins: Kube.Units.smallSpacing
                        }

                        Column {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                leftMargin:  Kube.Units.largeSpacing
                            }

                            Kube.Label{
                                id: subject
                                width: content.width - Kube.Units.gridUnit * 3
                                text: model.summary
                                color: delegateRoot.textColor
                                font.strikeout: model.complete
                                font.bold: model.doing && allViewButton.checked
                                maximumLineCount: 2
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                            }
                        }

                        Kube.Label {
                            id: date
                            anchors {
                                right: parent.right
                                bottom: parent.bottom
                            }
                            visible: model.date && !delegateRoot.buttonsVisible
                            text: Qt.formatDateTime(model.date, "dd MMM yyyy")
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

            color: "transparent"
            border.width: 1
            border.color: Kube.Colors.buttonColor

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

    Kube.Listener {
        filter: Kube.Messages.eventEditor
        onMessageReceived: eventPopup.createObject(root, message).open()
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
            TodoEditor {
                id: editor
                anchors.fill: parent
                accountId: root.currentAccount
                doing: doingViewButton.checked
                onDone: popup.close()
            }
        }
    }

}
