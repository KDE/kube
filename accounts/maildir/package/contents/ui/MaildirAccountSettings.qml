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

import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.framework.theme 1.0
import org.kube.accounts.maildir 1.0 as MaildirAccount


Rectangle {
    id: root
    property string accountId
    property string accountName
    property string icon

    color: ColorPalette.background

    GridLayout {
        id: gridLayout
        columns: 2
        Layout.fillWidth: true

        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: "General:"
        }

        Label { text: "Account Name" }
        TextField {
            id: name
            placeholderText: accountName
            Layout.fillWidth: true
            text: maildirSettings.accountName
            onTextChanged: { maildirSettings.accountName = text; }
        }

        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: "Maildir:"
        }

        Label { text: "Path" }
        RowLayout {
            TextField {
                id: path
                placeholderText: "path"
                Layout.fillWidth: true
                text: maildirSettings.path
                onTextChanged: { maildirSettings.path = text; }
                validator: maildirSettings.pathValidator
                Rectangle {
                    anchors.fill: parent
                    opacity: 0.2
                    color: path.acceptableInput ? "green" : "yellow"
                }
            }

            Button {
                iconName: "folder"
                onClicked:  {
                    fileDialogComponent.createObject(parent);
                }

                Component {
                    id: fileDialogComponent
                    FileDialog {
                        id: fileDialog
                        visible: true
                        title: "Please choose the maildir folder"

                        selectFolder: true

                        onAccepted: {
                            maildirSettings.path = fileDialog.fileUrl
                        }
                        onRejected: {
                        }
                    }
                }
            }
        }

        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: "Smtp:"
        }

        Label { text: "Username" }
        TextField {
            placeholderText: "Username"
            Layout.fillWidth: true
            text: maildirSettings.smtpUsername
            onTextChanged: { maildirSettings.smtpUsername = text; }
        }

        Label { text: "Password" }
        TextField {
            placeholderText: "Password"
            Layout.fillWidth: true
            text: maildirSettings.smtpPassword
            onTextChanged: { maildirSettings.smtpPassword = text; }
        }

        Label { text: "Server" }
        TextField {
            id: server
            placeholderText: "smtps://mainserver.example.net:465"
            Layout.fillWidth: true
            text: maildirSettings.smtpServer
            onTextChanged: { maildirSettings.smtpServer = text; }
            validator: maildirSettings.smtpServerValidator
            Rectangle {
                anchors.fill: parent
                opacity: 0.2
                color: server.acceptableInput ? "green" : "yellow"
            }
        }

        MaildirAccount.MaildirSettings {
            id: maildirSettings
            accountIdentifier: accountId
        }

        Button {
            text: "Save"
            onClicked: {
                maildirSettings.save();
            }
        }
        Button {
            text: "Remove"
            onClicked: {
                maildirSettings.remove();
            }
        }
    }
}
