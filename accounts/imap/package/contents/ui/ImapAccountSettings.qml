/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>

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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.1
import org.kube.framework 1.0 as Kube
import org.kube.accounts.imap 1.0 as ImapAccount

Item {

    property string accountId
    property string heading: "Connect your IMAP account"
    property string subheadline: "To let Kube access your account, fill in email address, username, password and give the account a title that will be displayed inside Kube. For information about which SMTP, IMAP address, which authentification and port to be used, please contact your email provider."

    ImapAccount.ImapSettings {
        id: imapSettings
        accountIdentifier: accountId
        accountType: "imap"
    }

    function save(){
        imapSettings.save()
    }

    function remove(){
        imapSettings.remove()
    }

    Item {
        anchors {
            fill: parent
        }

        GridLayout {
            anchors {
                fill: parent
            }
            columns: 2
            columnSpacing: Kube.Units.largeSpacing
            rowSpacing: Kube.Units.largeSpacing

            Kube.Label {
                text: "Title of Account"
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
                Layout.fillWidth: true
                placeholderText: "E.g. \"Work\", \"Home\" that will be displayed in Kube as name"
                text: imapSettings.accountName
                onTextChanged: {
                    imapSettings.accountName = text
                }
            }

            Kube.Label {
                text: "Name"
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
                Layout.fillWidth: true
                placeholderText: "Your name"
                text: imapSettings.userName
                onTextChanged: {
                    imapSettings.userName = text
                }
            }

            Kube.Label {
                text: "Email address"
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
                Layout.fillWidth: true

                text: imapSettings.emailAddress
                onTextChanged: {
                    imapSettings.emailAddress = text
                    imapSettings.imapUsername = text
                    imapSettings.smtpUsername = text
                }
                placeholderText: "Your email address"
            }

            Kube.Label {
                text: "Password"
                Layout.alignment: Qt.AlignRight
            }
            RowLayout {
                Layout.fillWidth: true

                Kube.TextField {
                    id: pwField
                    Layout.fillWidth: true

                    placeholderText: "Password of your email account"
                    text: imapSettings.imapPassword
                    onTextChanged: {
                        imapSettings.imapPassword = text
                        imapSettings.smtpPassword = text
                    }

                    echoMode: TextInput.Password
                }

                Controls.CheckBox {
                    text: "Show Password"
                    onClicked: {
                        if(pwField.echoMode == TextInput.Password) {
                            pwField.echoMode = TextInput.Normal;
                        } else {
                            pwField.echoMode = TextInput.Password;
                        }
                    }
                }
            }

            Kube.Label {
                text: "IMAP server address"
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
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

            Kube.Label {
                text: "Smtp address"
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
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
        }
    }
}
