/*
 * Copyright (C) 2015 Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.kube.mail 1.0 as Mail

Item {
    id: root
    property variant currentFolder
    SystemPalette { id: colorPalette; colorGroup: SystemPalette.Active }

    Item {
        id: searchBox

        width: root.width
        height: unit.size * 10

        TextField {
            anchors. centerIn: parent

            width: parent.width * 0.9

            placeholderText: "Search all email..."
        }
    }

    TreeView {
        id: treeView
        anchors {
            top: searchBox.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        TableViewColumn {
            title: "Name"
            role: "name"
            width: treeView.width - 5
        }
        model: Mail.FolderListModel { id: folderListModel }
        onCurrentIndexChanged: {
            model.fetchMore(currentIndex)
            root.currentFolder = model.data(currentIndex, Mail.FolderListModel.DomainObject)
        }
        backgroundVisible: false
        headerVisible: false
        style: TreeViewStyle {
            activateItemOnSingleClick: true
            rowDelegate: Rectangle {
                height: unit.size * 10
                color: "transparent"
            }
            itemDelegate: Rectangle {
                radius: 5
                border.width: 1
                border.color: "lightgrey"
                color: styleData.selected ? colorPalette.highlight : colorPalette.button
                PlasmaCore.IconItem {
                    id: iconItem
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: unit.size * 3
                    }
                    source: model.icon
                }
                Label {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: iconItem.right
                        leftMargin: unit.size * 3
                    }
                    renderType: Text.NativeRendering
                    text: styleData.value
                    font.pixelSize: 16
                    font.bold: true
                    color: styleData.selected ? colorPalette.highlightedText : colorPalette.text
                }
            }
            branchDelegate: Item {
                width: 16
                height: 16
                Text {
                    visible: styleData.column === 0 && styleData.hasChildren
                    text: styleData.isExpanded ? "\u25bc" : "\u25b6"
                    color: !control.activeFocus || styleData.selected ? styleData.textColor : "#666"
                    font.pointSize: 10
                    renderType: Text.NativeRendering
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: styleData.isExpanded ? 2 : 0
                }
            }
        }
    }
}
