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

VisualDataModel {
    id: mailDataModel
    property bool debug: true
    delegate: Rectangle {
        id: partColumn
        width: parent.width
        height: childrenRect.height
        Loader {
            id: partLoader
            anchors {
                top: parent.top
                left: parent.left
            }
            height: item? item.contentHeight : 0
            width: parent.width
        }
        Component.onCompleted: {
            switch (model.type) {
                case "PlainTextContent":
                case "Content":
                    partLoader.source = "TextContent.qml"
                    partLoader.item.debug = mailDataModel.debug
                    return;
                case "HtmlContent":
                    partLoader.source = "HtmlContent.qml"
                    return;
                case "Signature":
                    partLoader.source = "SignaturePart.qml"
                    partLoader.item.rootIndex = mailDataModel.modelIndex(index)
                    partLoader.item.debug = mailDataModel.debug
                    return;
                case "Encryption":
                    partLoader.source = "EncryptionPart.qml"
                    partLoader.item.rootIndex = mailDataModel.modelIndex(index)
                    partLoader.item.debug = mailDataModel.debug
                    return;
            }
            if (model.hasModelChildren) {
                partLoader.source = "MailPart.qml"
                partLoader.item.rootIndex = mailDataModel.modelIndex(index)
                partLoader.item.debug = mailDataModel.debug
            }
        }
    }
}
