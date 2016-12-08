/*
 * Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4 as Controls
import QtQuick.Controls 2.0 as Controls2
import org.kde.kirigami 1.0 as Kirigami
import QtQuick.Dialogs 1.0 as Dialogs

import org.kube.accounts.maildir 1.0 as MaildirAccount

Item {
    id: root

    //Controller
    MaildirAccount.MaildirController {
        id: accountsController
    }

    //Navigation
    Controls.ToolButton {
        iconName: "go-previous"

        tooltip: "go back"

        onClicked: {
            stack.pop()
        }
    }

    //Item to avoid anchors conflict with stack
    Item {

        anchors {
            fill: parent
            margins: Kirigami.Units.largeSpacing * 2
        }


        //BEGIN heading
        Kirigami.Heading {
            id: heading
            text: "Add your Maildir archive"

            color: Kirigami.Theme.highlightColor
        }

        Controls2.Label {
            id: subHeadline

            anchors {
                left: heading.left
                top: heading.bottom
            }

            width: parent.width

            text: "To let Kube access your maildir archive, add the path to your archive and give the account a title that will be displayed inside Kube"

            color: Kirigami.Theme.disabledTextColor
            wrapMode: Text.Wrap
        }
        //END heading

        GridLayout {
            anchors {
                top:subHeadline.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                topMargin: Kirigami.Units.largeSpacing * 2
            }

            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            Controls2.Label {
                text: "Title of account"
                Layout.alignment: Qt.AlignRight
            }
            Controls.TextField {
                id: title
                Layout.fillWidth: true

                text: accountsController.name

                onTextChanged: {
                    accountsController.name = text
                }
            }

            Controls2.Label {
                text: "Path"
                Layout.alignment: Qt.AlignRight
            }
            RowLayout {
                Layout.fillWidth: true

                Controls.TextField {
                    id: path
                    Layout.fillWidth: true

                    enabled: false

                    text: accountsController.path
                }

                Controls.Button {
                    iconName: "folder"

                    onClicked: {
                        fileDialogComponent.createObject(parent)
                    }

                    Component {
                        id: fileDialogComponent
                        Dialogs.FileDialog {
                            id: fileDialog

                            visible: true
                            title: "Choose the maildir folder"

                            selectFolder: true

                            onAccepted: {
                                accountsController.path = fileDialog.fileUrl
                            }
                        }
                    }
                }
            }

            /*
            Controls2.Label {
                text: ""
            }
            Controls.CheckBox {
                id: readOnly
                text: "Read only"
            }
            */

            Controls2.Label {
                text:  ""
                Layout.fillHeight: true
            }
            Controls2.Label {
                text: ""
            }

            Controls2.Label {
                text: ""
            }
            Item {
                Layout.fillWidth: true

                Controls2.Button {
                    text: "Discard"

                    onClicked: {
                        popup.close()
                    }
                }

                Controls2.Button {
                    id: saveButton

                    anchors.right: parent.right

                    text: "Save"

                    onClicked: {
                        accountsController.createAccount()
                        popup.close()
                    }
                }
            }
        }
    }
}

