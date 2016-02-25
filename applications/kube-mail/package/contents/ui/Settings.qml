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

Rectangle {
    id: root

    visible: false

    color: colorPalette.border

    opacity: 0.9

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.visible = false
        }
    }

    Rectangle {
        anchors.centerIn: parent

        height: root.height * 0.8
        width: root.width * 0.8

        color: colorPalette.background

        MouseArea {
            anchors.fill: parent
        }

        GridLayout {
            columns: 2
            anchors.fill: parent
            anchors.margins: 10
            rowSpacing: 10
            columnSpacing: 10

            Label { text: "Username" }
            TextField {
                id: username
                text: "username"
                Layout.fillWidth: true
            }

            Label { text: "Password" }
            TextField {
                id: password
                text: "password"
                Layout.fillWidth: true
            }

            Label { text: "Server" }
            TextField {
                id: server
                text: "server"
                Layout.fillWidth: true
            }

            KubeSettings.Settings {
                id: contextSettings
                identifier: "applicationcontext"
                property string currentAccountId: "current"
            }
            KubeSettings.Settings {
                id: accountSettings
                identifier: "account.current"
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

            Item {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Button {
                    id: button
                    anchors.centerIn: parent
                    text: "Save"
                    onClicked: {
                        contextSettings.save();
                        accountSettings.save();
                        identitySettings.save();
                        transportSettings.save();
                        root.visible = false;
                    }
                }
            }

        }
    }
}
