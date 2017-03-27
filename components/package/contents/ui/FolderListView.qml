/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework

Rectangle {
    id: root

    property variant currentFolder: null
    property variant accountId

    color: Kirigami.Theme.textColor

    KubeFramework.FolderController {
        id: folderController
        Binding on folder {
            //!! checks for the availability of the type
            when: !!root.currentFolder
            value: root.currentFolder
        }
    }

    Menu {
        id: contextMenu
        title: "Edit"

        MenuItem {
            text: "Synchronize"
            onTriggered: {
                folderController.synchronizeAction.execute()
            }
        }
    }

    TreeView {
        id: treeView

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        height: parent.height

        TableViewColumn {
            title: "Name"
            role: "name"
        }

        model: KubeFramework.FolderListModel {
            id: folderListModel
            accountId: root.accountId
        }

        onCurrentIndexChanged: {
            model.fetchMore(currentIndex)
            root.currentFolder = model.data(currentIndex, KubeFramework.FolderListModel.DomainObject)
        }

        alternatingRowColors: false
        headerVisible: false

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                var index = parent.indexAt(mouse.x, mouse.y)
                if (index.valid) {
                    folderController.folder = treeView.model.data(index, KubeFramework.FolderListModel.DomainObject)
                    contextMenu.popup()
                }
            }
        }

        style: TreeViewStyle {

            rowDelegate: Rectangle {
                color: styleData.selected ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

                height: Kirigami.Units.gridUnit * 1.5
                width: 20

            }

            frame: Rectangle {
                color: Kirigami.Theme.textColor
            }

            branchDelegate: Item {

                width: 16; height: 16

                Text  {

                    anchors.centerIn: parent

                    color: Kirigami.Theme.viewBackgroundColor
                    text: styleData.isExpanded ? "-" : "+"
                }

                //radius: styleData.isExpanded ? 0 : 100
            }

            itemDelegate: Rectangle {

                color: styleData.selected ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

                DropArea {
                    anchors.fill: parent

                    Rectangle {
                        anchors.fill: parent
                        color: Kirigami.Theme.viewBackgroundColor

                        opacity: 0.3

                        visible: parent.containsDrag
                    }
                    onDropped: {
                        folderController.folder = model.domainObject
                        folderController.mail = drop.source.mail
                        folderController.moveToFolderAction.execute()
                        drop.accept(Qt.MoveAction)
                        drop.source.visible = false
                    }
                }

                Row {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                    }
                    Text {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            leftMargin: Kirigami.Units.smallSpacing
                        }

                        text: styleData.value

                        color: Kirigami.Theme.viewBackgroundColor
                    }
                    ToolButton {
                        id: statusIcon
                        visible: false
                        iconName: ""
                        enabled: false
                        states: [
                            State {
                                name: "busy"; when: model.status == KubeFramework.FolderListModel.InProgressStatus
                                PropertyChanges { target: statusIcon; iconName: "view-refresh"; visible: styleData.selected }
                            },
                            State {
                                name: "error"; when: model.status == KubeFramework.FolderListModel.ErrorStatus
                                //The error status should only be visible for a moment, otherwise we'll eventually always show errors everywhere.
                                PropertyChanges { target: statusIcon; iconName: "emblem-error"; visible: styleData.selected }
                            },
                            State {
                                name: "checkmark"; when: model.status == KubeFramework.FolderListModel.SuccessStatus
                                //The success status should only be visible for a moment, otherwise we'll eventually always show checkmarks everywhere.
                                PropertyChanges { target: statusIcon; iconName: "checkmark"; visible: styleData.selected }
                            }
                        ]
                    }
                }
            }

            backgroundColor: Kirigami.Theme.textColor
            highlightedTextColor: Kirigami.Theme.highlightedTextColor
        }
    }
}
