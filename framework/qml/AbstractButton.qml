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
import QtQuick.Templates 2.0 as T
import org.kube.framework 1.0

T.Button {
    id: root

    property color color: Colors.buttonColor
    property color textColor: Colors.textColor
    property alias highlightColor: background.highlightColor
    property alias highlightOpacity: background.highlightOpacity
    property alias horizontalAlignment: label.horizontalAlignment
    property alias verticalAlignment: label.verticalAlignment
    property alias alert: background.alert

    width: Math.max(Units.gridUnit, contentItem.implicitWidth + leftPadding + rightPadding)
    height: contentItem.implicitHeight + topPadding + bottomPadding

    padding: Units.largeSpacing
    topPadding: Units.smallSpacing
    bottomPadding: Units.smallSpacing

    clip: true
    hoverEnabled: true
    Keys.onReturnPressed: root.clicked()

    background: DelegateBackground {
        id: background
        selected: root.checked
        focused: root.hovered || root.visualFocus
        color: root.color

        Rectangle {
            anchors.fill: parent
            visible: root.pressed
            color: Colors.textColor
            opacity: 0.2
        }
    }

    contentItem: Label {
        id: label
        text: root.text
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: root.enabled ? root.textColor : Colors.disabledTextColor
    }
}
