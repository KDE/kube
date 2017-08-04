/*
 *  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
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

import QtQuick 2.7
import QtQuick.Controls 2.2
import org.kube.framework 1.0 as Kube
import QtQuick.Layouts 1.3

Item {
    id: root
    default property alias children: menuLayout.children
    function close() {
        menu.close()
    }

    Component.onCompleted: {
        for (var i = 0; i < root.children.length; i++) {
            var child = root.children[i]
            child.clicked.connect(close)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: Kube.Colors.highlightColor
        border.width: 1
        visible: mouseArea.containsMouse || menu.visible
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.RightButton
        z: 1
        onClicked: {
            menu.x = mouseX
            menu.y = mouseY
            menu.open()
            mouse.accepted = true
        }
    }
    Menu {
        id: menu

        height: menuLayout.height
        width: menuLayout.width
        background: Rectangle {
            anchors.fill: parent
            color: Kube.Colors.backgroundColor
        }
        ColumnLayout {
            id: menuLayout
            width: childrenRect.width
            height: childrenRect.height
        }
    }
}

