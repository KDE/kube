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
    property string entityType
    property var roles
    property string sortRole
    property var filter: ({})

    property alias enabledEntities: entityFilterCollector.checkedEntities
    property var currentEntityIdentifier: null
    property var currentEntity: null

    signal entityCreated(var text, var accountId)
    signal entityRemoved(var entity)

    function clearSelection() {
        if (root.currentDelegate) {
            root.currentDelegate.clearSelection()
        }
    }

    function selectEntity(entity) {
        root.currentDelegate.selectEntity(entity)
    }

    Kube.CheckedEntities {
        id: entityFilterCollector
    }

    delegate: Kube.ListView {
        id: listView

        property bool editMode: false

        function clearSelection() {
            listView.currentIndex = -1
        }

        function selectEntity(entity) {
            if (entity) {
                var foundIndex = checkableModel.findIndex("object", entity)
                if (foundIndex >= 0) {
                    listView.currentIndex = foundIndex;
                }
            } else {
                listView.currentIndex = -1;
            }
        }

        function currentChanged(isCurrent) {
            if (isCurrent) {
                //Necessary to re-select folder on account change (so the tasklist is updated)
                if (currentItem) {
                    root.currentEntityIdentifier = currentItem.currentData.identifier
                    root.currentEntity = currentItem.currentData.object
                }
            }
        }

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
                    text: "+ " + qsTr("Add")
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

                    onAccepted: {
                        root.entityCreated(text, listView.parent.accountId)
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
                root.currentEntityIdentifier = currentItem.currentData.identifier
                root.currentEntity = currentItem.currentData.object
            }
        }

        model: Kube.CheckableEntityModel {
            id: checkableModel
            type: root.entityType
            roles: root.roles
            sortRole: root.sortRole
            filter: (listView.editMode ? root.filter : Object.assign({}, root.filter, {enabled: true}))
            accountId: listView.parent.accountId
            checkedEntities: entityFilterCollector
            onInitialItemsLoaded: {
                //Automatically enable edit mode if no entities are enabled
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
            property color color: !!model.color ? model.color : Kube.Colors.textColor
            property bool showColorIndicator: !!model.color

            DropArea {
                id: dropArea
                //Allows the source to find the target folder id on a drop event
                property var targetId: model.identifier
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

                        color: delegate.color

                        //Add a border because we don't have a background
                        border.color: Kube.Colors.backgroundColor
                        border.width: !delegate.showColorIndicator ? 1 : 0

                        Rectangle {
                            id: highlight
                            anchors.fill: parent
                            visible: checkBox.hovered || checkBox.visualFocus
                            color: Kube.Colors.highlightColor
                            opacity: 0.4
                        }

                        Kube.Icon {
                            anchors.centerIn: parent
                            height: Kube.Units.gridUnit - 4
                            width: Kube.Units.gridUnit - 4
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
                    visible: delegate.showColorIndicator && !listView.editMode
                    width: Kube.Units.gridUnit * 0.8
                    height: Kube.Units.gridUnit * 0.8
                    radius: width / 2
                    color: delegate.color
                }
                Kube.IconButton {
                    id: removeButton

                    visible: listView.editMode
                    onClicked: root.entityRemoved(model.object)
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
