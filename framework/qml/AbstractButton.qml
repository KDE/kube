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

    width: Math.max(Units.gridUnit, contentItem.implicitWidth + leftPadding + rightPadding)
    height: contentItem.implicitHeight + Units.smallSpacing * 2

    padding: Units.largeSpacing
    topPadding: Units.smallSpacing * 2
    bottomPadding: Units.smallSpacing *2

    clip: true
    hoverEnabled: true
    Keys.onReturnPressed: root.clicked()

    background: Rectangle {
        color: Colors.buttonColor

        border.width: 2
        border.color:  root.activeFocus && !root.pressed ? Colors.highlightColor : Colors.buttonColor

        Rectangle {
            anchors.fill: parent
            visible: root.hovered || root.pressed
            color: root.pressed ? Colors.textColor : Colors.viewBackgroundColor
            opacity: 0.2
        }
    }
}
