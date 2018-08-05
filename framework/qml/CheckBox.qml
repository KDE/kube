/*
 *  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
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
import QtQuick.Templates 2.2 as T
import org.kube.framework 1.0 as Kube

T.CheckBox {
    id: root

    implicitWidth: indicator.width
    implicitHeight: indicator.width

    Keys.onReturnPressed: root.toggle()

    indicator: Rectangle {
        width: Kube.Units.gridUnit
        height: Kube.Units.gridUnit

        color: Kube.Colors.viewBackgroundColor
        border.width: 1
        border.color: Kube.Colors.buttonColor

        Rectangle {
            id: highlight
            anchors.fill: parent
            visible: root.hovered || root.visualFocus
            color: Kube.Colors.highlightColor
            opacity: 0.4
        }

        Kube.Icon {
            anchors.centerIn: parent
            height: Kube.Units.gridUnit
            width: Kube.Units.gridUnit

            visible: root.checked
            iconName: Kube.Icons.checkbox
        }
    }
}
