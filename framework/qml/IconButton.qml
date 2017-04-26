/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Templates 2.0 as T
import org.kube.framework 1.0 as Kube

T.Button {
    id: root

    property var iconName

    width: Kube.Units.gridUnit + padding * 2
    height: Kube.Units.gridUnit + padding * 2

    padding: Kube.Units.smallSpacing

    clip: true
    hoverEnabled: true

    background: Rectangle {
        color: "#2980b9" //FIXME

        visible: root.hovered == true

        Rectangle {
            anchors.fill: parent
            visible: root.hovered || root.pressed
            color: root.pressed ? Kube.Colors.textColor : Kube.Colors.viewBackgroundColor
            opacity: 0.2
        }
    }

    contentItem: Kube.Icon {
        width: root.height - Kube.Units.smallSpacing * 2
        height: width
        iconName: root.iconName
    }
}

