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
import QtQuick.Layouts 1.1
import org.kube.framework 1.0 as Kube
import org.kube.accounts.generic 1.0 as GenericAccount

Item {

    property string accountId
    property string heading: qsTr("Connect your account")
    property string subheadline: qsTr("To let Kube access your account, fill in email address, username and the relevant server addresses. For information about the server details, please contact your email provider.")
    property bool valid: true
    implicitHeight: grid.implicitHeight

    GenericAccount.Settings {
        id: settings
        accountIdentifier: accountId
        accountType: "generic"
    }

    function save(){
        settings.save()
    }

    function remove(){
        settings.remove()
    }
    GridLayout {
        id: grid
        anchors.fill: parent
        columns: 2
        columnSpacing: Kube.Units.largeSpacing
        rowSpacing: Kube.Units.largeSpacing

        Kube.Label {
            text: qsTr("Name")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            Layout.fillWidth: true
            placeholderText: qsTr("Your name")
            text: settings.userName
            onTextChanged: {
                settings.userName = text
            }
        }

        Kube.Label {
            text: qsTr("Email address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            Layout.fillWidth: true

            text: settings.emailAddress
            onTextChanged: {
                settings.emailAddress = text
                settings.accountName = text
            }
            placeholderText: qsTr("Your email address")
        }
        Kube.Label {
            text: qsTr("Username")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            Layout.fillWidth: true

            text: settings.imapUsername
            onTextChanged: {
                settings.imapUsername = text
                settings.smtpUsername = text
                settings.carddavUsername = text
                settings.caldavUsername = text
            }
            placeholderText: qsTr("Your username for server access.")
        }

        Kube.Label {
            text: qsTr("IMAP address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            id: imapServer

            Layout.fillWidth: true

            placeholderText: "imaps://mainserver.example.net:993"
            text: settings.imapServer
            onTextChanged: {
                settings.imapServer = text
            }
            validator: settings.imapServerValidator
        }

        Kube.Label {
            text: qsTr("Use Starttls")
            Layout.alignment: Qt.AlignRight
        }
        Kube.CheckBox {
            Layout.fillWidth: true
            checked: settings.imapStarttls
            onToggled: settings.imapStarttls = checked
        }

        Kube.Label {
            text: qsTr("Authentication Method")
            Layout.alignment: Qt.AlignRight
        }
        Kube.ComboBox {
            Layout.fillWidth: true
            model: ["CLEARTEXT", "LOGIN", "PLAIN"]

            function getCurrentIndex(mode) {
                var index = find(mode)
                if (index < 0) {
                    //Default to PLAIN
                    return 2
                }
                return index
            }

            currentIndex: getCurrentIndex(settings.imapAuthenticationMode)
            onCurrentIndexChanged: settings.imapAuthenticationMode = textAt(currentIndex)
        }

        Kube.Label {
            text: qsTr("SMTP address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            Layout.fillWidth: true

            placeholderText: "smtps://mainserver.example.net:587"
            text: settings.smtpServer
            onTextChanged: {
                settings.smtpServer = text
            }
            validator: settings.smtpServerValidator
        }

        Kube.Label {
            text: qsTr("CardDAV address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            Layout.fillWidth: true

            placeholderText: "https://mainserver.example.net"
            text: settings.carddavServer
            onTextChanged: {
                settings.carddavServer = text
            }
        }

        Kube.Label {
            text: qsTr("CalDAV address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            Layout.fillWidth: true

            placeholderText: "https://mainserver.example.net"
            text: settings.caldavServer
            onTextChanged: {
                settings.caldavServer = text
            }
        }
    }
}
