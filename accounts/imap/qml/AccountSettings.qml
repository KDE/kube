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
import org.kube.accounts.imap 1.0 as ImapAccount

Item {

    property string accountId
    property string heading: qsTr("Connect your IMAP account")
    property string subheadline: qsTr("To let Kube access your account, fill in email address, username, password and give the account a title that will be displayed inside Kube. For information about which SMTP, IMAP address, which authentification and port to be used, please contact your email provider.")
    property bool valid: true
    implicitHeight: grid.implicitHeight

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
            text: imapSettings.userName
            onTextChanged: {
                imapSettings.userName = text
            }
        }

        Kube.Label {
            text: qsTr("Email address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            Layout.fillWidth: true

            text: imapSettings.emailAddress
            onTextChanged: {
                imapSettings.emailAddress = text
                imapSettings.imapUsername = text
                imapSettings.smtpUsername = text
                imapSettings.accountName = text
            }
            placeholderText: qsTr("Your email address")
        }

        Kube.Label {
            text: qsTr("IMAP server address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            id: imapServer

            Layout.fillWidth: true

            placeholderText: "imaps://mainserver.example.net:993"
            text: imapSettings.imapServer
            onTextChanged: {
                imapSettings.imapServer = text
            }
            validator: imapSettings.imapServerValidator
        }

        Kube.Label {
            text: qsTr("Smtp address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            id: smtpServer
            Layout.fillWidth: true

            placeholderText: "smtps://mainserver.example.net:993"
            text: imapSettings.smtpServer
            onTextChanged: {
                imapSettings.smtpServer = text
            }
            validator: imapSettings.smtpServerValidator
        }
    }
}
