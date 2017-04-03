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

    clip: true

    Text {
        id: text
        anchors.centerIn: parent
        color: root.textColor
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
