/*
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
import org.kube.components.theme 1.0 as KubeTheme

Rectangle {
    id: root

    signal clicked()
    property alias text: text.text
    property color textColor: KubeTheme.Colors.highlightedTextColor
    property string iconName: ""
    property alias implicitHeight: content.implicitHeight
    property alias implicitWidth: content.implicitWidth
    width: implicitWidth
    height: implicitHeight

    clip: true

    Row {
        id: content
        anchors.centerIn: parent
        spacing: KubeTheme.Units.smallSpacing
        Text {
            id: text
            anchors.verticalCenter: parent.verticalCenter
            color: root.textColor
        }
        Icon {
            id: icon
            anchors.verticalCenter: parent.verticalCenter
            iconName: root.iconName
            visible: iconName != ""
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
