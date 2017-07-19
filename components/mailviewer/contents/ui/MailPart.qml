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

import org.kube.framework 1.0 as Kube

Item {
    id: root
    property alias rootIndex: visualModel.rootIndex
    property alias model: visualModel.model
    property variant sender
    property variant date
    height: childrenRect.height

    MailDataModel {
        id: visualModel
    }

    Rectangle {
        id: border
        anchors {
            top: parent.top
            left: parent.left
            leftMargin: Kube.Units.smallSpacing
        }
        color: "lightgrey"
        height: partListView.height
        width: Kube.Units.smallSpacing
    }

    Text {
        id: sender
        anchors {
            left: border.right
            leftMargin: Kube.Units.smallSpacing
        }

        text: qsTr("sent by %1 on %2").arg(root.sender).arg(root.date)
        color: "grey"
    }
    ListView {
        id: partListView
        model: visualModel
        anchors {
            top: sender.bottom
            left: border.right
            margins: Kube.Units.smallSpacing
            leftMargin: Kube.Units.smallSpacing
        }
        spacing: 7
        height: contentHeight
        width: parent.width - Kube.Units.smallSpacing * 3
        interactive: false
    }
}
