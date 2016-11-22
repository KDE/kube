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
    id: encryption
    property alias rootIndex: visualModel.rootIndex
    property bool debug: true
    height: partListView.height
    width: parent.width

    MailDataModel {
        id: visualModel
        debug: encryption.debug
        model: messageParser.newTree
    }

    Column {
        id: partListView
        anchors {
            top: parent.top
            left: parent.left
        }
        width: parent.width
        spacing: 5
                Text {
            width: parent.width
            visible: encryption.debug
            text: model.type
        }
        Text {
            visible: model.errorType || encryption.debug
            text: model.errorType + ": " + model.errorString
        }
        BorderImage {
            width: parent.width
            height: childrenRect.height + 40
            border { left: 5; top: 5; right: 6; bottom: 6 }
            horizontalTileMode: BorderImage.Round
            verticalTileMode: BorderImage.Round

            source: /* "securityborders"+ */ model.securityLevel +".png"
            ListView {
                model: visualModel
                anchors {
                    top: parent.top
                    left: parent.left
                    margins: 20
                }
                height: contentHeight
                width: parent.width - 40

                spacing: 20

                interactive: false
            }
        }
    }
}
