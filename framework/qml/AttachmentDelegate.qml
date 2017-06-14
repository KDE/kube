/*
 * Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.7
import org.kube.framework 1.0 as Kube

Item {
    id: root

    property string name
    property string icon
    property alias actionIcon: actionButton.iconName
    signal clicked;
    signal execute;

    width: content.width + Kube.Units.smallSpacing
    height: content.height + Kube.Units.smallSpacing

    Rectangle {
        anchors.fill: parent

        id: background
        color: Kube.Colors.disabledTextColor
    }

    MouseArea {
        anchors.fill: parent

        onClicked: root.clicked()
    }

    Row {
        id: content
        anchors.centerIn: parent
        spacing: Kube.Units.smallSpacing
        Kube.Icon {
            anchors.verticalCenter: parent.verticalCenter
            height: Kube.Units.gridUnit
            width: Kube.Units.gridUnit
            iconName: root.icon
        }
        Label {
            anchors.verticalCenter: parent.verticalCenter
            text: root.name
            color: Kube.Colors.backgroundColor
        }
        Kube.IconButton {
            id: actionButton
            height: Kube.Units.gridUnit
            width: height
            onClicked: root.execute()
            padding: 0
        }
    }
}
