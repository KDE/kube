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
import QtQml.Models 2.2

DelegateModel {
    id: mailDataModel
    property bool debug: true
    function getPartType(type, hasModelChildren, forcePlain) {
        switch (type) {
            case "PlainTextContent":
            case "Content":
                return "plain";
            case "HtmlContent":
                if (forcePlain) {
                    return "plain";
                }
                return "html";
            case "Signature":
                return "signature";
            case "Encryption":
                return "encryption";
        }
        if (hasModelChildren) {
            return "envelope";
        }
        return "";
    }

    delegate: Item {
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
            var isHtml = false
            var forcePlain = false
            var partType = getPartType(model.type, model.hasModelChildren, forcePlain);

            switch (partType) {
                case "plain":
                    partLoader.setSource("TextContent.qml",
                                        {"isHtml": isHtml,
                                        "content": model.content,
                                        "embedded": model.embeded,
                                        "type": model.type,
                                        "debug": debug})
                    break
                case "html":
                    partLoader.setSource("HtmlContent.qml",
                                        {"content": model.content,
                                        "debug": debug})
                    break;
                case "signature":
                    partLoader.setSource("SignaturePart.qml",
                                        {"rootIndex": mailDataModel.modelIndex(index),
                                        "securityLevel": model.securityLevel,
                                        "type": model.type,
                                        "debug": debug})
                    break;
                case "encryption":
                    partLoader.setSource("EncryptionPart.qml",
                                        {"rootIndex": mailDataModel.modelIndex(index),
                                        "securityLevel": model.securityLevel,
                                        "type": model.type,
                                        "model": mailDataModel.model,
                                        "errorType": model.errorType,
                                        "errorString": model.errorString,
                                        "debug": debug})
                    break;
                case "envelope":
                    partLoader.setSource("MailPart.qml",
                                        {"rootIndex": mailDataModel.modelIndex(index),
                                        "model": mailDataModel.model,
                                        "debug": debug})
                    break;
            }
        }
    }
}
