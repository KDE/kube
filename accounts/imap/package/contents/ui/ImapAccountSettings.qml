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

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.accounts.imap 1.0 as ImapAccount


Item {

    property string accountId

    ImapAccount.ImapSettings {
             id: imapSettings
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
            text: "Connect your IMAP account"

            color: Kirigami.Theme.highlightColor
        }

        Label {
            id: subHeadline

            anchors {
                left: heading.left
                top: heading.bottom
            }

            width: parent.width

            text: "To let Kube access your account, fill in email address, username, password and give the account a title that will be displayed inside Kube. For information about which SMTP, IMAP server, which authentification and port to be used, please contact your email provider"
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
                topMargin: Kirigami.Units.largeSpacing
                bottomMargin: Kirigami.Units.largeSpacing * 2
            }

            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            Kirigami.Label {
                text: "Title of Acocunt"
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                Layout.fillWidth: true

                text: imapSettings.accountName
                onTextChanged: {
                    imapSettings.accountName = text
                }
            }

            Kirigami.Label {
                text: "Email address"
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                Layout.fillWidth: true

                text: imapSettings.emailAddress
                onTextChanged: {
                    imapSettings.emailAddress = text
                }
            }

            Kirigami.Label {
                text: "Username"
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                Layout.fillWidth: true

                text: imapSettings.imapUsername
                onTextChanged: {
                    imapSettings.imapUsername = text
                    imapSettings.smtpUsername = text
                }
            }

            Kirigami.Label {
                text: "Password"
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                Layout.fillWidth: true

               text: imapSettings.imapPassword
               onTextChanged: {
                   imapSettings.imapPassword = text
                   imapSettings.smtpPassword = text
                }
            }

            Kirigami.Label {
                text: "IMAP address"
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                id: imapServer

                Layout.fillWidth: true

                placeholderText: "imaps://mainserver.example.net:993"
                text: imapSettings.imapServer
                onTextChanged: {
                    imapSettings.imapServer = text
                }
                validator: imapSettings.imapServerValidator

                Rectangle {
                    anchors.fill: parent
                    opacity: 0.2
                    color: "yellow"
                    visible: imapServer.acceptableInput
                }
            }

            Kirigami.Label {
                text: "Smtp address"
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                id: smtpServer
                Layout.fillWidth: true

                placeholderText: "smtps://mainserver.example.net:993"
                text: imapSettings.smtpServer
                onTextChanged: {
                    imapSettings.smtpServer = text
                }
                validator: imapSettings.smtpServerValidator

                Rectangle {
                    anchors.fill: parent
                    opacity: 0.2
                    color: "yellow"
                    visible: smtpServer.acceptableInput
                }
            }

            Label {
                text: ""
            }
            Item {
                Layout.fillWidth: true

                Button {
                    text: "Delete"

                    onClicked: {
                        imapSettings.remove()
                        root.closeDialog()
                    }
                }

                Button {
                    anchors.right: parent.right

                    text: "Save"

                    onClicked: {
                        focus: true
                        imapSettings.save()
                        root.closeDialog()
                    }
                }
            }
        }
    }
}
