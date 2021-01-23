/*
 * Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import QtQuick.Controls 2.0
import org.kube.framework 1.0 as Kube
import org.kube.test 1.0

ApplicationWindow {
    id: app
    height: 900
    width: 1500
    //Pass as --file=$filename argument
    property string filepath: ""

    Component.onCompleted: {
        app.filepath = Qt.application.arguments[2].replace("--file=", "")
        var initialState = {
            accounts: [
                {
                    id: "account1",
                    name: "Test Account"
                }
            ],
            identities: [{
                account: "account1",
                name: "Test Identity",
                address: "identity@example.org"
            }],
            resources: [
                {
                    id: "caldavresource",
                    account: "account1",
                    type: "caldav",
                },
                {
                    id: "caldavresource2",
                    account: "account2",
                    type: "caldav",
                }
            ],
            calendars: [{
                id: "calendar1",
                resource: "caldavresource",
                name: "Test Calendar",
                color: "#af1a6a",
                contentTypes: ["event"],
                enabled: true
            },
            {
                id: "calendar16",
                resource: "caldavresource",
                name: "Test Calendar16",
                color: "#f67400",
                contentTypes: ["event"],
                enabled: true
            },
            {
                id: "account2calendar",
                resource: "caldavresource2",
                name: "Account2Calendar",
                color: "#f67400",
                contentTypes: ["event"],
                enabled: true
            }],
        }
        TestStore.setup(initialState)
        Kube.Context.currentAccountId = "account1"
    }

    Kube.File {
        id: file
        path: app.filepath
    }
    Kube.MessageParser {
        id: messageParser
        message: file.data
    }

    Rectangle {
        width: 800
        height: 800
        anchors.centerIn: parent
        border.color: "blue"
        border.width: 2
        MailPartView {
            anchors.fill: parent
            visible: true
            model: messageParser.parts
        }
    }
}
