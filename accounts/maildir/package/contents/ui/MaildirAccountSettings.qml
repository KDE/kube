/*
 *   Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.framework.theme 1.0
import org.kube.accounts.maildir 1.0 as MaildirAccount


Item {
    property string accountId

    MaildirAccount.MaildirSettings {
        id: maildirSettings
        accountIdentifier: accountId
    }

    anchors.fill: parent

    Item {
        anchors {
            fill: parent
            margins: Kirigami.Units.largeSpacing * 2
        }

        Kirigami.Heading {
            id: heading
            text: "Add your Maildir archive"

            color: Kirigami.Theme.highlightColor
        }

        Label {
            id: subHeadline

            anchors {
                left: heading.left
                top: heading.bottom
            }

            width: parent.width

            text: "To let Kube access your maildir archive, add the path to your archive and give the account a title that will be displayed inside Kube"
            //TODO wait for kirgami theme disabled text color
            opacity: 0.5
            wrapMode: Text.Wrap
        }

        GridLayout {
            anchors {
                top:subHeadline.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                topMargin: Kirigami.Units.largeSpacing * 2
                bottomMargin: Kirigami.Units.largeSpacing * 2
            }

            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            Kirigami.Label {
                text: "Display tilte"
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                Layout.fillWidth: true

                text: maildirSettings.accountName
                onTextChanged: {
                    maildirSettings.accountName = text
                }
            }

            Kirigami.Label {
                text: "Path"
                Layout.alignment: Qt.AlignRight
            }
            RowLayout {
                Layout.fillWidth: true

                TextField {
                    id: path
                    Layout.fillWidth: true

                    text: maildirSettings.path
                    onTextChanged: {
                        maildirSettings.path = text
                    }
                    validator: maildirSettings.pathValidator

                    Rectangle {
                        anchors.fill: parent
                        opacity: 0.2
                        color: "yellow"
                        visible: path.acceptableInput == false
                    }
                }

                Button {
                    iconName: "folder"

                    onClicked: {
                        fileDialogComponent.createObject(parent)
                    }

                    Component {
                        id: fileDialogComponent
                        FileDialog {
                            id: fileDialog

                            visible: true
                            title: "Choose the maildir folder"

                            selectFolder: true

                            onAccepted: {
                                maildirSettings.path = fileDialog.fileUrl
                            }
                        }
                    }
                }
            }

            Label {
                text: ""
            }
            CheckBox {
                text: "Read only"
            }

            Label {
                text:  ""
                Layout.fillHeight: true
            }
            Label {
                text: ""
            }

            Label {
                text: ""
            }
            Item {
                Layout.fillWidth: true

                Button {
                    text: "Delete"

                    onClicked: {
                        maildirSettings.remove()
                        root.closeDialog()
                    }
                }

                Button {
                    id: saveButton

                    anchors.right: parent.right

                    text: "Save"

                    onClicked: {
                        focus: true
                        maildirSettings.save()
                        root.closeDialog()
                    }
                }
            }
        }
    }
}