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

import org.kde.kube.settings 1.0 as KubeSettings
import org.kde.kube.accounts.maildir 1.0 as MaildirAccount

Rectangle {
    id: root
    property string accountId
    property string accountName: "Maildir"

    color: colorPalette.background

    GridLayout {
        id: gridLayout
        columns: 2
        Layout.fillWidth: true

        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: "Account: " + accountName
        }

        Label { text: "Path" }
        TextField {
            id: path
            placeholderText: "path"
            Layout.fillWidth: true
        }

        Text {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: "Smtp:"
        }

        Label { text: "Username" }
        TextField {
            id: username
            placeholderText: "username"
            Layout.fillWidth: true
        }

        Label { text: "Password" }
        TextField {
            id: password
            placeholderText: "password"
            Layout.fillWidth: true
        }

        Label { text: "Server" }
        TextField {
            id: server
            placeholderText: "server"
            Layout.fillWidth: true
        }

        MaildirAccount.MaildirSettings {
            id: maildirSettings
            identifier: "org.kde.maildir.instance1"
            property alias path: path.text
        }

        KubeSettings.Settings {
            id: accountSettings
            identifier: "account." + modelData
            property string primaryIdentity: "current"
        }
        KubeSettings.Settings {
            id: identitySettings
            identifier: "identity.current"
            property string transport: "current"
        }
        KubeSettings.Settings {
            id: transportSettings
            identifier: "transport.current"
            property alias username: username.text
            property alias password: password.text
            property alias server: server.text
        }

        Button {
            id: button
            text: "Save"
            onClicked: {
                transportSettings.save();
                maildirSettings.save();
            }
        }
    }
}
