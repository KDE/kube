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

    property string searchString: ""
    property bool autoLoadImages: false

    delegate: Item {
        id: partColumn

        width: parent ? parent.width : 0
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

        function getDetails(signatureDetails)
        {
            var details = "";
            if (signatureDetails.noSignaturesFound) {
                details += qsTr("This message has been signed but we failed to validate the signature.") + "\n"
            } else if (signatureDetails.keyMissing) {
                details += qsTr("This message has been signed using the key %1.").arg(signatureDetails.keyId) + "\n";
                details += qsTr("The key details are not available.") + "\n";
            } else {
                details += qsTr("This message has been signed using the key %1 by %2.").arg(signatureDetails.keyId).arg(signatureDetails.signer) + "\n";
                if (signatureDetails.keyRevoked) {
                    details += qsTr("The key was revoked.") + "\n"
                }
                if (signatureDetails.keyExpired) {
                    details += qsTr("The key has expired.") + "\n"
                }
                if (signatureDetails.keyIsTrusted) {
                    details += qsTr("You are trusting this key.") + "\n"
                }
                if (!signatureDetails.signatureIsGood && !signatureDetails.keyRevoked && !signatureDetails.keyExpired && !signatureDetails.keyIsTrusted) {
                    details += qsTr("The signature is invalid.") + "\n"
                }
            }
            return details
        }

        Column {
            id: buttons
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.rightMargin: Kube.Units.smallSpacing
            spacing: Kube.Units.smallSpacing
            Kube.IconButton {
                id: encryptedButton
                width: Kube.Units.gridUnit
                height: width
                iconName: Kube.Icons.secure
                color: getColor(model.encryptionSecurityLevel)
                backgroundOpacity: 0.5
                visible: model.encrypted
                tooltip: model.encryptionDetails.keyId == "" ? qsTr("This message is encrypted but we don't have the key for it.") : qsTr("This message is encrypted to the key: %1").arg(model.encryptionDetails.keyId);

                //FIXME make text copyable
                // Kube.SelectableItem {
                //     visualParent: encryptedButton
                //     text: parent.tooltip
                // }
            }
            Kube.IconButton {
                id: signedButton
                width: Kube.Units.gridUnit
                height: width
                iconName: Kube.Icons.signed
                color: getColor(model.signatureSecurityLevel)
                backgroundOpacity: 0.5
                visible: model.signed
                tooltip: getDetails(model.signatureDetails)
            }
        }
        Rectangle {
            id: border
            visible: encryptedButton.hovered || signedButton.hovered
            anchors.topMargin: Kube.Units.smallSpacing
            anchors.top: buttons.bottom
            anchors.bottom: partLoader.bottom
            anchors.right: buttons.right
            width: Kube.Units.smallSpacing
            color: getColor(model.securityLevel)
            opacity: 0.5
        }

        Loader {
            id: partLoader
            anchors {
                top: parent.top
                left: buttons.right
                leftMargin: Kube.Units.smallSpacing
                right: parent.right
            }
            height: item ? item.contentHeight : 0
            width: parent.width
            Binding {
                target: partLoader.item
                property: "searchString"
                value: root.searchString
                when: partLoader.status == Loader.Ready
            }
            Binding {
                target: partLoader.item
                property: "autoLoadImages"
                value: root.autoLoadImages
                when: partLoader.status == Loader.Ready
            }
        }
        Component.onCompleted: {
            switch (model.type) {
                case "plain":
                    partLoader.setSource("TextPart.qml",
                                        {"content": model.content,
                                        "embedded": model.embedded,
                                        "type": model.type
                                        })
                    break
                case "html":
                    partLoader.setSource("HtmlPart.qml",
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
                case "ical":
                    partLoader.setSource("ICalPart.qml",
                                        {"content": model.content})
                    break;
            }
        }
    }
}
