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
import org.kube.accounts.imap 1.0 as ImapAccount


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
            text: imapSettings.accountName
            onTextChanged: { imapSettings.accountName = text; root.accountName = text; }
        }

        Label { text: "User Name" }
        TextField {
            placeholderText: "Your Name"
            Layout.fillWidth: true
            text: imapSettings.userName
            onTextChanged: { imapSettings.userName = text; }
        }

        Label { text: "Email Address" }
        TextField {
            placeholderText: "Your EMail Address"
            Layout.fillWidth: true
            text: imapSettings.emailAddress
            onTextChanged: { imapSettings.emailAddress = text; }
        }

        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: "Imap:"
        }

        Label { text: "Username" }
        TextField {
            placeholderText: "Username"
            Layout.fillWidth: true
            text: imapSettings.imapUsername
            onTextChanged: { imapSettings.imapUsername = text; }
        }

        Label { text: "Password" }
        TextField {
            placeholderText: "Password"
            Layout.fillWidth: true
            text: imapSettings.imapPassword
            onTextChanged: { imapSettings.imapPassword = text; }
        }

        Label { text: "Server" }
        TextField {
            id: server
            placeholderText: "imaps://mainserver.example.net:993"
            Layout.fillWidth: true
            text: imapSettings.imapServer
            onTextChanged: { imapSettings.imapServer = text; }
            validator: imapSettings.imapServerValidator
            Rectangle {
                anchors.fill: parent
                opacity: 0.2
                color: server.acceptableInput ? "green" : "yellow"
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
            text: imapSettings.smtpUsername
            onTextChanged: { imapSettings.smtpUsername = text; }
        }

        Label { text: "Password" }
        TextField {
            placeholderText: "Password"
            Layout.fillWidth: true
            text: imapSettings.smtpPassword
            onTextChanged: { imapSettings.smtpPassword = text; }
        }

        Label { text: "Server" }
        TextField {
            id: server
            placeholderText: "smtps://mainserver.example.net:465"
            Layout.fillWidth: true
            text: imapSettings.smtpServer
            onTextChanged: { imapSettings.smtpServer = text; }
            validator: imapSettings.smtpServerValidator
            Rectangle {
                anchors.fill: parent
                opacity: 0.2
                color: server.acceptableInput ? "green" : "yellow"
            }
        }

        ImapAccount.ImapSettings {
            id: imapSettings
            accountIdentifier: accountId
        }

        Button {
            text: "Save"
            onClicked: {
                imapSettings.save();
            }
        }
        Button {
            text: "Remove"
            onClicked: {
                imapSettings.remove();
            }
        }
    }
}
