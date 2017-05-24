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
    signal open;
    signal download;

    width: content.width + Kube.Units.gridUnit / 2
    height: content.height + Kube.Units.gridUnit / 2

    Rectangle {
        anchors.fill: parent

        id: background
        color: Kube.Colors.disabledTextColor
    }

    Row {
        id: content

        anchors.centerIn: parent

        spacing: Kube.Units.smallSpacing

        Rectangle {
            id: mimetype

            height: Kube.Units.gridUnit
            width: Kube.Units.gridUnit

            color: Kube.Colors.backgroundColor

            Kube.Icon {
                height: parent.height
                width: height
                iconName: root.icon
            }
        }

        Label {
            text: root.name
            color: Kube.Colors.backgroundColor
        }
        Kube.IconButton {
            height: parent.height
            width: height
            iconName: Kube.Icons.goDown
            onClicked: root.download()
        }
        Kube.IconButton {
            height: parent.height
            width: height
            iconName: Kube.Icons.edit
            onClicked: root.open()
        }
    }
}
