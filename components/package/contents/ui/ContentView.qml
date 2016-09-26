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
import QtQuick.Controls 1.3

Item {
    id: root
    property int nestingLevel;
    property bool isHtml;
    property string content;
    property string contentType;
    property int contentWidth: contentLoader.item.contentWidth
    property int contentHeight: contentLoader.item.contentHeight
    Rectangle {
        id: contentRect

        //Only for development
        // border.width: 1
        // border.color: "black"
        // radius: 5
        // anchors.leftMargin: nestingLevel * 5
        anchors.fill: parent

        Loader {
            id: contentLoader
            property string content: root.content
            anchors.fill: parent
            sourceComponent: isHtml ? htmlComponent : textComponent
        }

        Component {
            id: textComponent
            TextView {
                content: root.content
            }
        }
        Component {
            id: htmlComponent
            WebView {
                content: root.content
            }
        }
    }
}
