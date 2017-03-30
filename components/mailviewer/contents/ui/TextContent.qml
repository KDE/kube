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

import QtQuick 2.7
import org.kube.components.theme 1.0 as KubeTheme

Item {
    id: textItem

    property bool debug: true

    width: partColumn.width
    height: textColumn.height

    Column {
        id: textColumn

        anchors {
            top: parent.top
            left: parent.left
        }

        width: parent.width
        spacing: 5

        TextEdit  {
            width: parent.width

            readOnly: true
            selectByMouse: true

            text: model.content
            wrapMode: Text.WordWrap

            color: model.embeded ? KubeTheme.Colors.diabledTextColor : KubeTheme.Colors.textColor
        }

        //BEGIN debug
        Text {
            width: parent.width
            visible: textItem.debug
            text: model.type
        }
        //END debug
    }
}
