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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework


Item {
    id: root

    function loadMessage(message, loadAsDraft) {
        composer.loadMessage(message, loadAsDraft)
    }

    function send() {
        composer.send()
    }

    function saveAsDraft() {
        composer.saveAsDraft()
    }

    function clear() {
        composer.clear();
    }

    KubeFramework.ComposerController {
        id: composer
    }

    ColumnLayout {

        anchors.fill: parent

        GridLayout {

            columns: 2

            Kirigami.Label {
                text: "From"
            }

            ComboBox {
                id: identityCombo
                model: composer.identityModel
                textRole: "displayName"

                Layout.fillWidth: true

                onCurrentIndexChanged: {
                    composer.currentIdentityIndex = currentIndex
                }
            }

            Kirigami.Label {
                text: "To"
            }

            RowLayout {
                Layout.fillWidth: true

                TextField {
                    id: to

                    Layout.fillWidth: true

                    text: composer.to

                    onTextChanged: {
                        composer.to = text;
                    }
                }

                Button {
                    id: ccButton

                    text: "Cc"

                    onClicked: {
                        cc.visible = true
                        ccButton.visible = false
                    }
                }

                Button {
                    id: bccButton

                    text: "Bcc"

                    onClicked: {
                        bcc.visible = true
                        bccButton.visible = false
                    }
                }
            }

            Kirigami.Label {
                text: "Cc"

                visible: cc.visible
            }

            TextField {
                id: cc

                Layout.fillWidth: true

                visible: false

                text: composer.cc

                onTextChanged: {
                        composer.cc = text;
                }
            }

            Kirigami.Label {
                text: "Bcc"

                visible: bcc.visible
            }

            TextField {
                id: bcc

                Layout.fillWidth: true

                visible : false

                text: composer.bcc

                onTextChanged: {
                        composer.bcc = text;
                }
            }
        }

        TextField {
            id: subject

            Layout.fillWidth: true

            placeholderText: "Enter Subject"

            text: composer.subject

            onTextChanged: {
                composer.subject = text;
            }
        }

        Item {

            Layout.fillWidth: true

            height: subject.height * 1.5

            Button {

                anchors {
                    bottom: parent.bottom
                }

                text: "Save as Draft"

                onClicked: {
                    composer.saveAsDraft()
                }
            }

            Button {

                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }

                text: "Attach"

                onClicked: {
                    fileDialog.open();
                }
            }
        }

        RowLayout {

            Layout.fillWidth: true

            Repeater {

                model: composer.attachments

                delegate: Kirigami.Label {
                    id: name

                    text: modelData

                    Rectangle {

                        anchors.fill: parent

                        color: "lightsteelblue"
                    }
                }
            }
        }

        TextArea {
            id: content

            text: composer.body

            onTextChanged: {
                composer.body = text;
            }

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    FileDialog {
        id: fileDialog

        title: "Please choose a file"

        onAccepted: {
            console.log("You chose: " + fileDialog.fileUrl)
            composer.addAttachment(fileDialog.fileUrl);
        }
    }
}
