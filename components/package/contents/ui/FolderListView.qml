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

    property variant currentFolder
    property variant accountId

    color: Kirigami.Theme.textColor

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

        model: KubeFramework.FolderListModel { id: folderListModel} //; accountId: wrapper.accountId }

        onCurrentIndexChanged: {
            model.fetchMore(currentIndex)
            root.currentFolder = model.data(currentIndex, KubeFramework.FolderListModel.DomainObject)
        }

        alternatingRowColors: false
        headerVisible: false

        style: TreeViewStyle {

            rowDelegate: Rectangle {
                color: styleData.selected ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

                height: Kirigami.Units.gridUnit * 2
                width: 20

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

               Text {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: Kirigami.Units.smallSpacing
                    }

                    text: "#" + styleData.value

                    color: Kirigami.Theme.viewBackgroundColor
                }
            }

            backgroundColor: Kirigami.Theme.textColor
            highlightedTextColor: Kirigami.Theme.highlightedTextColor
        }
    }
}
