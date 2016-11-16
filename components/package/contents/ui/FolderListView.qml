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

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework

Item {
    id: root

    property variant currentFolder
    property variant accountId

    TreeView {
        anchors.fill: parent
        id: treeView
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
    }
}
