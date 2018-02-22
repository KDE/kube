/*
 * Copyright (C) 2018 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import QtQuick.Window 2.0

import org.kube.framework 1.0 as Kube
import org.kube.test 1.0
import "qml"

ApplicationWindow {
    id: app
    height: Screen.desktopAvailableHeight * 0.8
    width: Screen.desktopAvailableWidth * 0.8

    Component.onCompleted: {
        var initialState = {
            accounts: [{
                    id: "account1",
                    name: "Test Account"
                }],
            identities: [{
                    account: "account1",
                    name: "Test Identity",
                    address: "identity@example.org"
                }],
            resources: [{
                    id: "resource1",
                    account: "account1",
                    type: "dummy"
                },
                {
                    id: "resource2",
                    account: "account1",
                    type: "mailtransport"
                }],
            folders: [{
                    id: "folder1",
                    resource: "resource1",
                    name: "Folder 1",
                    mails: [{
                            resource: "resource1",
                            messageId: "<msg1@test.com>",
                            date: "2017-07-24T15:46:29",
                            subject: "subject1",
                            body: "body",
                            to: ["to@example.org"],
                            cc: ["cc@example.org"],
                            bcc: ["bcc@example.org"],
                            unread: true
                        },
                        {
                            resource: "resource1",
                            inReplyTo: "<msg1@test.com>",
                            date: "2017-07-24T16:46:29",
                            subject: "subject2",
                            body: "body2",
                            to: ["to@example.org"],
                            unread: true
                        },
                        {
                            resource: "resource1",
                            inReplyTo: "<msg1@test.com>",
                            date: "2017-07-24T17:46:29",
                            subject: "subject3",
                            body: "body3\n\n\n\nfoo\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nThe End",
                            to: ["to@example.org"],
                            unread: true
                        },
                        {
                            resource: "resource1",
                            inReplyTo: "<msg1@test.com>",
                            date: "2017-07-24T18:46:29",
                            subject: "subject4",
                            body: "body4",
                            to: ["to@example.org"],
                            unread: false
                        },
                        {
                            resource: "resource1",
                            date: "2017-07-20T18:46:29",
                            subject: "UTF-8 Madness Umlauts:öüä Snowflake:❆ Heart:♥",
                            body: "UTF-8 Madness Umlauts:öüä Snowflake:❆ Heart:♥",
                            to: ["öüä@example.org"],
                            unread: true
                        },
                    ]
                }],
        }
        TestStore.setup(initialState)
    }

    View {
        anchors.fill: parent
    }
}
