/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
 *  Copyright (C) 2019 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import QtQuick.Controls 2
import QtQuick.Layouts 1.2

import org.kube.framework 1.0 as Kube

Kube.InlineAccountSwitcher {
    id: root

    property bool selectionEnabled: false
    property string contentType: "event"

    property alias enabledCalendars: calendarFilterCollector.checkedEntities
    property var currentCalendar: null

    function clearSelection() {
        if (root.currentDelegate) {
            root.currentDelegate.clearSelection()
        }
    }

    Kube.CheckedEntities {
        id: calendarFilterCollector
    }

    delegate: Kube.ListView {
        id: listView

        function clearSelection() {
            listView.currentIndex = -1
        }

        function currentChanged() {
            if (listView.parent.isCurrent) {
                //Necessary to re-select folder on account change (so the tasklist is updated)
                if (currentItem) {
                    root.currentCalendar = currentItem.currentData.identifier
                }
            }
        }

        property bool editMode: false
        property Component buttonDelegate: Kube.IconButton {
            height: Kube.Units.gridUnit
            padding: 0
            iconName: Kube.Icons.overflowMenu_inverted
            onClicked: listView.editMode = !listView.editMode;
            checkable: true
            checked: listView.editMode
        }

        footer: Component {
            FocusScope {
                height: Kube.Units.gridUnit +  Kube.Units.smallSpacing * 2
                width: parent.width
                visible: listView.editMode

                Kube.TextButton {
                    id: button
                    text: "+ " + qsTr("Add calendar")
                    textColor: Kube.Colors.highlightColor
                    focus: true
                    onClicked: {
                        lineEdit.visible = true
                        lineEdit.forceActiveFocus()
                    }
                }

                Kube.TextField {
                    id: lineEdit
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    visible: false

                    signal aborted()

                    selectByMouse: true

                    onEditingFinished: {
                        accepted()
                    }

                    validator: RegExpValidator { regExp: /.+/ }

                    Keys.onReturnPressed: {
                        if (acceptableInput) {
                            accepted()
                        } else {
                            aborted()
                        }
                    }

                    Keys.onEscapePressed: {
                        aborted()
                    }

                    placeholderText: button.text

                    Kube.EntityController {
                        id: entityController
                    }

                    onAccepted: {
                        entityController.create({type: "calendar", account: listView.parent.accountId, entity: {
                            "name": text,
                            "color": Kube.Colors.jazzberryJam,
                            "contentTypes": [contentType],
                            "enabled": true,
                        }})
                        clear()
                        visible = false
                        button.forceActiveFocus(Qt.TabFocusReason)
                    }
                    onAborted: {
                        clear()
                        visible = false
                        button.forceActiveFocus(Qt.TabFocusReason)
                    }
                }
            }

        }
        footerPositioning: ListView.InlineFooter

        currentIndex: -1

        Layout.fillWidth: true
        Layout.maximumHeight: Math.min(contentHeight, parent.height - Kube.Units.gridUnit)
        Layout.preferredHeight: contentHeight

        onCurrentItemChanged: {
            if (currentItem) {
                root.currentCalendar = currentItem.currentData.identifier
            }
        }

        model: Kube.CheckableEntityModel {
            id: calendarModel
            type: "calendar"
            roles: ["name", "color", "enabled"]
            sortRole: "name"
            filter: listView.editMode ? {"contentTypes": contentType} : {"contentTypes": contentType, enabled: true}
            accountId: listView.parent.accountId
            checkedEntities: calendarFilterCollector
            onInitialItemsLoaded: {
                //Automatically enable edit mode if no calendars are enabled
                if (rowCount() == 0) {
                    listView.editMode = true;
                }
            }

        }
        delegate: Kube.ListDelegate {
            id: delegate

            selectionEnabled: root.selectionEnabled

            width: listView.availableWidth
            height: Kube.Units.gridUnit * 1.5
            hoverEnabled: true
            property bool isActive: listView.currentIndex === index

            DropArea {
                id: dropArea
                property var calendarId: model.identifier
                anchors.fill: parent

                Rectangle {
                    anchors.fill: parent
                    color: Kube.Colors.highlightColor
                    opacity: 0.3
                    visible: parent.containsDrag
                }

                onDropped: drop.accept(Qt.MoveAction)
            }

            background: Kube.DelegateBackground {
                anchors.fill: parent
                color: Kube.Colors.textColor
                focused: delegate.activeFocus || delegate.hovered
                selected: isActive
            }

            RowLayout {
                anchors {
                    fill: parent
                    leftMargin: Kube.Units.smallSpacing
                }
                spacing: Kube.Units.smallSpacing
                Kube.CheckBox {
                    id: checkBox
                    opacity: 0.9
                    visible: listView.editMode
                    checked: model.checked || model.enabled
                    onCheckedChanged: {
                        model.checked = checked
                        model.enabled = checked
                    }

                    indicator: Rectangle {
                        width: Kube.Units.gridUnit * 0.8
                        height: Kube.Units.gridUnit * 0.8

                        color: model.color

                        Rectangle {
                            id: highlight
                            anchors.fill: parent
                            visible: checkBox.hovered || checkBox.visualFocus
                            color: Kube.Colors.highlightColor
                            opacity: 0.4
                        }

                        Kube.Icon {
                            anchors.centerIn: parent
                            height: Kube.Units.gridUnit
                            width: Kube.Units.gridUnit
                            visible: checkBox.checked
                            iconName: Kube.Icons.checkbox_inverted
                        }
                    }

                }
                Kube.Label {
                    id: label
                    Layout.fillWidth: true
                    text: model.name
                    color: Kube.Colors.highlightedTextColor
                    elide: Text.ElideLeft
                    clip: true
                }
                Rectangle {
                    visible: !listView.editMode
                    width: Kube.Units.gridUnit * 0.8
                    height: Kube.Units.gridUnit * 0.8
                    radius: width / 2
                    color: model.color
                }
                Kube.IconButton {
                    id: removeButton

                    Kube.EntityController {
                        id: entityController
                    }

                    visible: listView.editMode
                    onClicked: entityController.remove(model.object)
                    padding: 0
                    iconName: Kube.Icons.remove
                }
                ToolTip {
                    id: toolTipItem
                    visible: delegate.hovered && label.truncated
                    text: label.text
                }
            }
        }
    }
}
