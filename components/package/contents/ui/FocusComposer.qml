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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as Controls2

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework

Controls2.Popup {
    id: root

    //Controler
    KubeFramework.ComposerController {
        id: composer
    }

    //BEGIN functions
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
    //END functions

    modal: true
    focus: true
    closePolicy: Controls2.Popup.CloseOnEscape | Controls2.Popup.CloseOnPressOutsideParent

    Item {

        height: parent.height
        width: parent.width

        ColumnLayout {

            anchors {
                fill: parent
                margins: Kirigami.Units.largeSpacing
            }

            ColumnLayout {

                anchors.fill: parent

                Controls2.TextField {
                    id: subject

                    Layout.fillWidth: true

                    placeholderText: "Enter Subject..."

                    text: composer.subject

                    onTextChanged: {
                        composer.subject = text;
                    }
                }

                GridLayout {

                    columns: 2

                    Controls2.Label {
                        text: "To"
                    }

                    AutocompleteLineEdit {
                        id: to

                        Layout.fillWidth: true

                        text: composer.to
                        onTextChanged: {
                            composer.to = text;
                        }

                        model: composer.recepientAutocompletionModel
                        onSearchTermChanged: {
                            composer.recepientSearchString = searchTerm
                        }
                    }


                    Controls2.Label {
                        text: "Cc"

                        visible: cc.visible
                    }

                    AutocompleteLineEdit {
                        id: cc

                        Layout.fillWidth: true

                        visible: false

                        text: composer.cc

                        onTextChanged: {
                            composer.cc = text;
                        }

                        model: composer.recepientAutocompletionModel
                        onSearchTermChanged: {
                            composer.recepientSearchString = searchTerm
                        }
                    }

                    Controls2.Label {
                        text: "Bcc"

                        visible: bcc.visible
                    }

                    AutocompleteLineEdit {
                        id: bcc

                        Layout.fillWidth: true

                        visible : false

                        text: composer.bcc

                        onTextChanged: {
                            composer.bcc = text;
                        }

                        model: composer.recepientAutocompletionModel
                        onSearchTermChanged: {
                            composer.recepientSearchString = searchTerm
                        }
                    }
                }

                RowLayout {
                    Controls2.Label {
                        text: "Sending as"
                    }

                    Controls2.ComboBox {
                        id: identityCombo
                        model: composer.identityModel
                        textRole: "displayName"

                        Layout.fillWidth: true

                        onCurrentIndexChanged: {
                            composer.currentIdentityIndex = currentIndex
                        }
                    }

                    Controls2.Button {
                        id: ccButton

                        text: "Cc"

                        onClicked: {
                            cc.visible = true
                            ccButton.visible = false
                        }
                    }

                    Controls2.Button {
                        id: bccButton

                        text: "Bcc"

                        onClicked: {
                            bcc.visible = true
                            bccButton.visible = false
                        }
                    }
                }

                /*
                Item {

                    Layout.fillWidth: true

                    height: subject.height * 1.5

                    Controls2.Button {

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

                        delegate: Controls2.Label {
                            id: name

                            text: modelData

                            Rectangle {

                                anchors.fill: parent

                                color: "lightsteelblue"
                            }
                        }
                    }
                }
                */

                Controls2.TextArea {
                    id: content

                    text: composer.body

                    onTextChanged: {
                        composer.body = text;
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                RowLayout {
                    id: bottomBar

                    width: parent.width

                    Controls2.Button {

                        text: "Discard"

                        onClicked: {
                            root.close()
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }


                    Controls2.Button {

                        text: "Save as Draft"

                        onClicked: {
                            saveAsDraft()
                            root.close()
                        }
                    }

                    Controls2.Button {
                        text: "Send"

                        onClicked: {
                            send()
                            root.close()
                        }
                    }
                }
            }
        }
    }
}
