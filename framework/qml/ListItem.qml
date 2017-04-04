/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.4

Item {
    id: delegateRoot

    readonly property bool isCurrentItem: ListView.isCurrentItem

    height: Unit.width * 25
    width: parent.width

    MouseArea {
        id: mouseArea

        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent

        color: colorPalette.background

        //clickColor
        Rectangle {
            id: clickColor

            anchors.fill: parent

            color: colorPalette.selected
            opacity: 0.4

            visible: mouseArea.pressed
        }

        //border
        Rectangle {

            anchors.bottom: parent.bottom

            height: 1
            width: parent.width

            color: colorPalette.border
            opacity: 0.2
        }
    }
}
