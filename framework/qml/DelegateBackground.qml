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

Rectangle {
    id: root
    property bool focused: false
    property bool selected: false
    property color highlightColor: Kube.Colors.highlightColor
    property alias highlightOpacity: highlight.opacity
    property color borderColor: Kube.Colors.focusedButtonColor

    Rectangle {
        anchors.fill: parent
        visible: root.selected
        color: root.highlightColor
    }
    Rectangle {
        id: highlight
        anchors.fill: parent
        visible: root.focused && !root.selected
        color: root.highlightColor
        opacity: 0.4
    }
    Rectangle {
        anchors.fill: parent
        visible: root.focused && root.selected
        border.color: root.borderColor
        border.width: 2
        color: "transparent"
    }
}
