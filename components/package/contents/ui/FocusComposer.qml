/*
 *  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as Controls2

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.actions 1.0 as KubeAction

Controls2.Popup {
    id: root

    //Controller
    KubeFramework.ComposerController {
        id: composer
        onDone: {
            root.close()
        }
    }

    //context
    property variant mailcontext: composer.mailContext

    //actions
    property variant sendAction: composer.sendAction
    property variant saveAsDraftAction: composer.saveAsDraftAction

    //BEGIN functions
    function loadMessage(message, loadAsDraft) {
        composer.loadMessage(message, loadAsDraft)
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

                GridLayout {

                    columns: 2

                    Controls2.Label {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        text: "To"
                    }

                    AutocompleteLineEdit {
                        id: to

                        Layout.fillWidth: true

                        text: mailcontext.to
                        onTextChanged: {
                            mailcontext.to = text;
                        }

                        model: composer.recepientAutocompletionModel
                        onSearchTermChanged: {
                            composer.recepientSearchString = searchTerm
                        }
                    }


                    Controls2.Label {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        text: "Cc"
                        visible: cc.visible
                    }

                    AutocompleteLineEdit {
                        id: cc

                        Layout.fillWidth: true

                        visible: false

                        text: mailcontext.cc

                        onTextChanged: {
                            mailcontext.cc = text;
                        }

                        model: composer.recepientAutocompletionModel
                        onSearchTermChanged: {
                            composer.recepientSearchString = searchTerm
                        }
                    }

                    Controls2.Label {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        text: "Bcc"
                        visible: bcc.visible
                    }

                    AutocompleteLineEdit {
                        id: bcc

                        Layout.fillWidth: true

                        visible : false

                        text: mailcontext.bcc

                        onTextChanged: {
                            mailcontext.bcc = text;
                        }

                        model: composer.recepientAutocompletionModel
                        onSearchTermChanged: {
                            composer.recepientSearchString = searchTerm
                        }
                    }

                    Controls2.Label {
                        text: "From"
                    }

                    RowLayout {

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
                }

                Controls2.TextField {
                    id: subject

                    Layout.fillWidth: true

                    placeholderText: "Enter Subject..."

                    text: mailcontext.subject

                    onTextChanged: {
                        mailcontext.subject = text;
                    }
                }

                Controls2.TextArea {
                    id: content

                    text: mailcontext.body

                    onTextChanged: {
                        mailcontext.body = text;
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

                        enabled: saveAsDraftAction.ready
                        onClicked: {
                            saveAsDraftAction.execute()
                        }
                    }

                    Controls2.Button {
                        text: "Send"

                        enabled: sendAction.ready
                        onClicked: {
                            sendAction.execute()
                        }
                    }
                }
            }
        }
    }
}
