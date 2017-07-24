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
    property variant currentData: model
    property var listView: root.parent
    default property alias contentItem: root.children

    height: Kube.Units.gridUnit * 3
    width: listView.width

    border.color: Kube.Colors.buttonColor
    border.width: 1
    color: listView.currentIndex == root.index ? Kube.Colors.highlightColor : Kube.Colors.viewBackgroundColor

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: listView.currentIndex = root.index
    }
}
