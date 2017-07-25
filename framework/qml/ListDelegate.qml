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
import org.kube.framework 1.0 as Kube
import QtQuick.Templates 2.0 as T

T.ItemDelegate {
    id: root
    property variant currentData: model
    property alias color: background.color
    property alias border: background.border
    property color textColor: ListView.isCurrentItem ? Kube.Colors.highlightedTextColor : Kube.Colors.textColor

    height: Kube.Units.gridUnit * 3
    width: root.ListView.view.width
    hoverEnabled: true

    background: Rectangle {
        id: background
        border.color: Kube.Colors.buttonColor
        border.width: 1
        color: Kube.Colors.viewBackgroundColor
        Rectangle {
            anchors.fill: parent
            visible: root.ListView.isCurrentItem
            color: Kube.Colors.highlightColor
        }
        Rectangle {
            anchors.fill: parent
            visible: root.hovered || root.activeFocus
            border.color: Kube.Colors.focusedButtonColor
            border.width: 2
            color: "transparent"
        }
    }
}
