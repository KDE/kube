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

    property alias enabledCalendars: calendarFilterCollector.checkedEntities
    property bool editMode: false
    property string contentType: "event"

    Kube.CheckedEntities {
        id: calendarFilterCollector
    }

    buttonDelegate: Kube.IconButton {
        height: Kube.Units.gridUnit
        padding: 0
        iconName: Kube.Icons.overflowMenu_inverted
        onClicked: root.editMode = !root.editMode;
        checkable: true
        checked: root.editMode
    }

    delegate: Kube.ListView {
        id: listView
        Layout.fillWidth: true
        Layout.maximumHeight: Math.min(contentHeight, parent.height - Kube.Units.gridUnit)
        Layout.preferredHeight: contentHeight
        spacing: Kube.Units.smallSpacing
        model: Kube.CheckableEntityModel {
            id: calendarModel
            type: "calendar"
            roles: ["name", "color", "enabled"]
            sortRole: "name"
            filter: root.editMode ? {"contentTypes": contentType} : {"contentTypes": contentType, enabled: true}
            accountId: listView.parent.accountId
            checkedEntities: calendarFilterCollector
            onInitialItemsLoaded: {
                //Automatically enable edit mode if no calendars are enabled
                if (rowCount() == 0) {
                    root.editMode = true;
                }
            }

        }
        delegate: Kube.ListDelegate {
            id: delegate
            width: listView.availableWidth
            height: Kube.Units.gridUnit
            hoverEnabled: true
            background: Item {}
            RowLayout {
                anchors.fill: parent
                spacing: Kube.Units.smallSpacing
                Kube.CheckBox {
                    id: checkBox
                    opacity: 0.9
                    visible: root.editMode
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
                    visible: !root.editMode
                    width: Kube.Units.gridUnit * 0.8
                    height: Kube.Units.gridUnit * 0.8
                    radius: width / 2
                    color: model.color
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
