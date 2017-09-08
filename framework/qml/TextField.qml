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
import QtQuick.Templates 2.0 as T
import org.kube.framework 1.0 as Kube

T.TextField {
    id: root

    property alias backgroundColor: background.color
    property alias backgroundOpacity: background.opacity

    implicitHeight: Kube.Units.gridUnit + Kube.Units.smallSpacing * 2
    implicitWidth: Kube.Units.gridUnit * 5 + Kube.Units.smallSpacing * 2

    padding: Kube.Units.smallSpacing

    color: Kube.Colors.textColor
    font.family: Kube.Font.fontFamily
    selectionColor: Kube.Colors.highlightColor
    selectByMouse: true

    Label {
        id: placeholder
        x: root.leftPadding
        y: root.topPadding

        width: root.width - (root.leftPadding + root.rightPadding)
        height: root.height - (root.topPadding + root.bottomPadding)

        visible: root.text == ""
        text: root.placeholderText
        color: Kube.Colors.disabledTextColor
        elide: Text.ElideRight
    }

    background: Rectangle {
        id: background
        color: Kube.Colors.viewBackgroundColor
        border.width: 1
        border.color: root.activeFocus ? Kube.Colors.highlightColor : Kube.Colors.buttonColor
    }
}
