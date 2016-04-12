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

import org.kde.kirigami 1.0 as Kirigami
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.theme 1.0

Item {
    id: root
    property variant currentFolder
    SystemPalette { id: colorPalette; colorGroup: SystemPalette.Active }

    Item {
        id: searchBox

        width: root.width
        height: Unit.size * 10

        TextField {
            anchors. centerIn: parent

            width: parent.width * 0.9

            placeholderText: "Search all email..."
        }
    }

    Rectangle {
        anchors {
            top: searchBox.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        color: "white"
        ScrollView {
            anchors.fill: parent
            ListView {
                id: listView
                anchors.fill: parent
                delegate: accountDelegate
                model: KubeFramework.AccountsModel { id: accountsModel }
            }
        }
    }

    Component {
        id: accountDelegate

        Item {
            id: wrapper

            property var accountId: model.accountId

            width: listView.width
            height: 30

            Rectangle {
                id: headerView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top

                height: 30

                color: "#333"
                border.color: Qt.lighter(color, 1.2)
                Kirigami.Icon {
                    id: iconItem
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 4
                    source: model.icon
                }
                Text {
                    anchors.left: iconItem.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 4

                    font.pixelSize: parent.height-4
                    color: '#fff'

                    text: name
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (parent.state != "expanded") {
                        parent.state = "expanded";
                    } else {
                        parent.state = ""
                    }
                }
            }

            Item {
                id: folderView

                anchors.top: headerView.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                opacity: 0
                visible: false

                Rectangle {
                    anchors.fill: parent
                    TreeView {
                        anchors.fill: parent
                        id: treeView
                        TableViewColumn {
                            title: "Name"
                            role: "name"
                            width: treeView.width - 5
                        }
                        model: KubeFramework.FolderListModel { id: folderListModel; accountId: wrapper.accountId }
                        onCurrentIndexChanged: {
                            model.fetchMore(currentIndex)
                            root.currentFolder = model.data(currentIndex, KubeFramework.FolderListModel.DomainObject)
                        }
                        backgroundVisible: false
                        headerVisible: false
                        style: TreeViewStyle {
                            activateItemOnSingleClick: true
                            rowDelegate: Rectangle {
                                height: Unit.size * 10
                                color: "transparent"
                            }
                            itemDelegate: Rectangle {
                                radius: 5
                                border.width: 1
                                border.color: "lightgrey"
                                color: styleData.selected ? colorPalette.highlight : colorPalette.button
                                Kirigami.Icon {
                                    id: iconItem
                                    anchors {
                                        verticalCenter: parent.verticalCenter
                                        left: parent.left
                                        leftMargin: Unit.size * 3
                                    }
                                    source: model.icon
                                }
                                Label {
                                    anchors {
                                        verticalCenter: parent.verticalCenter
                                        left: iconItem.right
                                        leftMargin: Unit.size * 3
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
            }

            states: [
                State {
                    name: "expanded"

                    PropertyChanges { target: wrapper; height: listView.height - accountsModel.rowCount() * 30 }
                    PropertyChanges { target: folderView; opacity: 1; visible: true }
                    PropertyChanges { target: wrapper.ListView.view; contentY: wrapper.y; interactive: false }
                }
            ]

            transitions: [
                Transition {
                    NumberAnimation {
                        duration: 150;
                        properties: "height,width,anchors.rightMargin,anchors.topMargin,opacity,contentY"
                    }
                }
            ]
        }
    }

}
