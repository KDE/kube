/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsystems.com>

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
import QtQuick.Controls 2
import QtQuick.Controls 1 as Controls1
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQml.Models 2.2

import org.kube.framework 1.0 as Kube

Flickable {
    id: root
    property variant accountId

    ScrollBar.vertical: ScrollBar {}
    clip: true
    contentWidth: root.width
    contentHeight: contentItem.childrenRect.height
    Kube.ScrollHelper {
        id: scrollHelper
        anchors.fill: root
        flickable: root
    }

    Kube.TreeView {
        id: treeView
        anchors {
            left: parent.left
            right: parent.right
        }
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        height: __listView.contentItem.height

        Kube.MouseProxy {
            anchors.fill: parent
            target: scrollHelper
            forwardWheelEvents: true
        }

        Controls1.TableViewColumn {
            title: "Name"
            role: "name"
        }

        model: Kube.FolderListModel {
            id: folderListModel
            accountId: root.accountId
        }

        onActivated: {
            //TODO do some event compression in case of double clicks
            model.fetchMore(currentIndex);
            Kube.Fabric.postMessage(Kube.Messages.folderSelection, {"folder": model.data(index, Kube.FolderListModel.DomainObject),
                                                                    "trash": model.data(index, Kube.FolderListModel.Trash)});
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"folder": model.data(index, Kube.FolderListModel.DomainObject)});
        }


        onDropped: {
            Kube.Fabric.postMessage(Kube.Messages.moveToFolder, {"mail": drop.source.mail, "folder": model.domainObject})
            drop.accept(Qt.MoveAction)
            drop.source.visible = false
        }
    }
}
