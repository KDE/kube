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

import QtQuick 2.8
import QtQuick.Templates 2.1 as T
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

    background: Rectangle {
        color: root.pressed ? Colors.highlightColor : Colors.positiveColor

        Rectangle {
            anchors.fill: parent
            visible: root.hovered
            color: Colors.viewBackgroundColor
            opacity: 0.1
        }
    }

    contentItem: Text {
        text: root.text
        //TODO font
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: Colors.highlightedTextColor
    }
}
