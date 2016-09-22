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
    id: root

    height: partColumn.height + 40
    width: delegateRoot.width

    Column {
        id: partColumn

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 20
        }

        spacing: 10

        Repeater {
            model: content

            delegate: Column {
                id: delegateRoot

                width: partColumn.width

                Loader {
                    id: loader
                }

                Component.onCompleted: {

                    switch (model.type) {
                        case "encrypted":
                            loader.source = "EncryptedPart.qml";
                            break;
                        case "embeded":
                            loader.source = "EmbededPart.qml";
                            break;
                        case "frame":
                            loader.source = "Frame.qml"
                            break;
                        case "plaintext":
                            loader.source = "TextPart.qml";
                            break;
                        case "html":
                            loader.source = "HtmlPart.qml";
                        break;
                    }
                }
            }
        }


        Item {
            id: footer

            height: 5
            width: 10
        }
    }
}
