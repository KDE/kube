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
import org.kube.framework 1.0 as Kube

DelegateModel {
    id: root

    delegate: Item {
        id: partColumn

        width: parent.width
        height: childrenRect.height

        function getColor(securityLevel)
        {
            if (securityLevel == "good") {
                return Kube.Colors.positiveColor
            }
            if (securityLevel == "bad") {
                return Kube.Colors.negativeColor
            }
            if (securityLevel == "notsogood") {
                return Kube.Colors.warningColor
            }
            return Kube.Colors.lightgrey
        }

        Row {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: partLoader.height
            spacing: Kube.Units.smallSpacing
            Rectangle {
                id: border
                visible: model.encrypted || model.signed
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }
                width: Kube.Units.smallSpacing
                color: getColor(model.securityLevel)
                opacity: 0.5
            }

            Loader {
                id: partLoader
                anchors {
                    top: parent.top
                }
                height: item? item.contentHeight : 0
                width: parent.width
            }
        }
        Component.onCompleted: {
            switch (model.type) {
                case "plain":
                    partLoader.setSource("TextContent.qml",
                                        {"content": model.content,
                                        "embedded": model.embeded,
                                        "type": model.type
                                        })
                    break
                case "html":
                    partLoader.setSource("HtmlContent.qml",
                                        {"content": model.content,
                                        })
                    break;
                case "error":
                    partLoader.setSource("ErrorPart.qml",
                                        {
                                        "errorType": model.errorType,
                                        "errorString": model.errorString,
                                        })
                    break;
                case "encapsulated":
                    partLoader.setSource("MailPart.qml",
                                        {"rootIndex": root.modelIndex(index),
                                        "model": root.model,
                                        "sender": model.sender,
                                        "date": model.date
                                        })
                    break;
            }
        }
    }
}
