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
                    specialpurpose: ["drafts"],
                    mails: [{
                            resource: "resource1",
                            messageId: "<msg1@test.com>",
                            date: "2017-07-24T15:46:29",
                            subject: "subject1",
                            body: "body",
                            to: ["to@example.org"],
                            cc: ["No Quoting <cc@example.org>"],
                            bcc: ["\"Needs, Quoting\" <bcc@example.org>"],
                            draft: true
                        },
                        {
                            resource: "resource1",
                            messageId: "<msg2@test.com>",
                            date: "2017-07-23T15:46:29",
                            subject: "LooooooooooooooooooooooooooooooooooooooooooooooooooooooooonggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggEnd",
                            body: "LooooooooooooooooooooooooooooooooooooooooooooooooooooooooonggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggEnd\nbody\nbody\n\n\nbody\n\n\nbody\n\n\nbody\n\n\nbody\nbody\n\n\nbody\n\n\nbody\n\n\nbody\n\n\nbody\n\n\n\n\n\n\n\n\nbody\nbody\n\n\nbody\n\n\nbody\n\n\nbody\n\n\nbody\n\n\nbody",
                            to: ["toLoooooooooooooooooooooooooooooooooooooooooooooooooong@example.org"],
                            cc: ["ccLoooooooooooooooooooooooooooooooooooooooooooooooooong@example.org"],
                            bcc: ["bccLoooooooooooooooooooooooooooooooooooooooooooooooooong@example.org"],
                            draft: true
                        },
                        {
                            resource: "resource1",
                            messageId: "<msg3@test.com>",
                            date: "2017-07-23T16:46:29",
                            subject: "HTML",
                            body: "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'Noto Sans'; font-size:9pt; font-weight:400; font-style:normal;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">body</span></p></body></html>",
                            to: ["html@example.org"],
                            draft: true
                        },
                        {
                            resource: "resource1",
                            messageId: "<msg4@test.com>",
                            date: "2017-07-24T16:46:29",
                            subject: "A more pleasant writing experience ",
                            body: "Lots of studies suggest the optimal line length for reading (and therefore writing) is somewhere between 50 and 75 characters per line. For most writing purposes I suspect the real figure is between 60 and 70 (and it will depend on your screen and your font so you will need to fiddle to get that right).\n\nI agree this works a lot better.",
                            to: ["html@example.org"],
                            draft: true
                        }

                    ]
                }],
        }
        TestStore.setup(initialState)
    }

    View {
        anchors.fill: parent
    }
}
