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
                },
                {
                    id: "account2",
                    name: "Test Account2"
                }
            ],
            identities: [{
                    account: "account1",
                    name: "Test Identity",
                    address: "identity@example.org"
                }
            ],
            resources: [{
                    id: "resource1",
                    account: "account1",
                    type: "dummy"
                },
                {
                    id: "resource2",
                    account: "account1",
                    type: "mailtransport"
                },
                {
                    id: "resource3",
                    account: "account2",
                    type: "dummy"
                }
            ],
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
                        {
                            resource: "resource1",
                            date: "2017-07-20T17:47:29",
                            subject: "PlainLongLine",
                            body: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                            to: ["to@example.org"],
                            unread: true
                        },
                        {
                            resource: "resource1",
                            date: "2017-07-20T17:46:29",
                            subject: "HTMLLongLine",
                            body: "<pre>Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n<pre>",
                            bodyIsHtml: true,
                            to: ["to@example.org"],
                            unread: true
                        },
                        {
                            resource: "resource1",
                            date: "2017-07-20T17:46:29",
                            subject: "ComplexHTMLLongLine",
                            //We assume that @media trigger the complex html view
                            body: "<pre>Hi Mélanie,\n\nI'm sorry @media to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n<pre>",
                            bodyIsHtml: true,
                            to: ["to@example.org"],
                            unread: true
                        },
                        {
                            resource: "resource1",
                            date: "2017-07-20T17:47:29",
                            subject: "WithAttachment",
                            body: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                            to: ["to@example.org"],
                            unread: true,
                            attachments: [
                                {
                                    name: "myImage.png",
                                    mimeType: "image/png",
                                    data: "no real data",
                                }
                            ],
                        },
                        {
                            resource: "resource1",
                            date: "2017-07-20T17:47:29",
                            subject: "WithBadPKeyAttachment",
                            body: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                            to: ["to@example.org"],
                            unread: true,
                            attachments: [
                                {
                                    name: "myKey.asc",
                                    mimeType: "application/pgp-keys",
                                    data: "no real data",
                                }
                            ],
                        },
                        {
                            resource: "resource1",
                            date: "2017-07-20T17:47:29",
                            subject: "WithGoodPKeyAttachment",
                            body: "Hi Mélanie,\n\nI'm sorry to start this on such late notice, but we'd like to get Foo and boo to woo next week, because the following weeks are unfortunately not possible for us.\n",
                            to: ["to@example.org"],
                            unread: true,
                            attachments: [
                                {
                                    name: "myKey.asc",
                                    mimeType: "application/pgp-keys",
                                    data:
"-----BEGIN PGP PUBLIC KEY BLOCK-----

mQENBEr9ij4BCADaFvyhzV7IrCAr/sCvfoPerAd4dYIGTeCeBmInu3p4oEG0rXTB
2zL2t9zd7jVwCmYLsqb0Y4+7UIulBTSVa/SxsFkxPIzQaGd+CYpIpCl2P7oXBQH/
365i/gvng4UTb5CytBp9MToft2tUgqvK/LD30fBWbWVE1ohmuGYDviJesuqJGeRG
KPOmjRk8LcXecZoNAnahy6y/rHPQzbC7LVazrWfdYCsZ1w202kwwLAPj0aNO6d4n
M9NYo26/mB+5+odJ5gbxfdKWQQOFCha8UzEXbZzjJsRNFhUyuEDEd2gBlhDm3jrY
ACT3u1adLJ1GsY6biN3u1IEUwi/7+uofZRPXABEBAAG0K3VuaXR0ZXN0IGtleSAo
bm8gcGFzc3dvcmQpIDx0ZXN0QGtvbGFiLm9yZz6JATgEEwECACIFAkr9ij4CGwMG
CwkIBwMCBhUIAgkKCwQWAgMBAh4BAheAAAoJEI2YYMWPJG3mxggH/iDnmHhKI40r
bPvDPSMVFz4pNL5oYrGMjOUIz5ibjn9N19Fz/T5kxupbYVRbdcx6kRy4uQd97sJ8
4985JkHEr/TSHne5p0F+tQLKq+WcJST+cbvkFR9m9WTZISOo+bP/rKGsf6GOGfl/
vzObv8tF0E8Yy0Lu1lYdynnBRygT+VKt5GzcNzsS3Af1kgrnoQ1gVWjKueSR32hJ
BILfOpQlKP/RdrOND1N0uljaJBQsUmYDJ5Gd+YL0VX4/56tfqt4gcuqhiD+Vz6BG
+55gqwuFK4/o2gawPELjOLUy5dh/b6MDvWehbasRPcyT1fFm9YY6iku4ZEx8EzLv
IJKiXLAx1+i5AQ0ESv2KPgEIAO6+rYyBG0YBfacSx36VCrzvRe8V0CqmUkzIHZJ0
EN/s95yCQwG0yC3M0KRGDzTeCXRik68h/qdw3KEgfZzu4rJAj9w/J4JMtcuhuCYL
rL4iP32hvLfqZDqwBaRCmlEkqArF0Jahb5SW3cPYZlE+I9I2V1xYX3bSZ7jcihAx
VWtkheYtZcDY3u/7cWZNUauGNKRh4E0+ToCBI+erEd5EPCQDQrL/e5pEj+s/+Coy
BvJeQdAPX/wjfYVe8t+5GDLqOvpbUBWJWUptv/oTd3wOtJCwwr/OWNeXf7ipgtoG
KpJgr+FHLOEb3cXtF1YPzwpTOs/J/bv3JdGyQ3Kx1BlTUzUAEQEAAYkBHwQYAQIA
CQUCSv2KPgIbDAAKCRCNmGDFjyRt5h0nB/40FPmVWhD2ok3opPRTwYMzUAOHkgMU
k2bJfIH185hMvnHLAPCgUMr8xvjcx3NphiRCaC6mabIxHI9hDAbi6uyDBNTyQtm2
sl/r1vqjFcxX49l9yt0AgMy3284IdwK9xdlwMLY/MbCL9GKe/D6RmZ6i/4wpxHdP
9X3cGh66UW09NUO1Gria0isRfwf/OxkV+KxA7qxX2bWOHS3noUAj7I43MJCvTuAP
gTIgEfjdpx1C2Tv97SxoLZ4t6raztvmwqIyCQIuzukD0H9JGFjWT9bGY7obPl7hO
Bvr+rojxTJ3X+pzb2LJQwJnALL/VdIF3yHtGu2//Yfu4oxGGA0M90KiW
=an8Y
-----END PGP PUBLIC KEY BLOCK-----
",
                                }
                            ],
                        },
                    ]
                },
                {
                    id: "folder2",
                    resource: "resource1",
                    name: "Folder 2"
                }
            ],
        }
        TestStore.setup(initialState)
    }

    Shortcut {
        onActivated: Kube.Fabric.postMessage(Kube.Messages.search, {})
        sequence: StandardKey.Find
    }
    View {
        anchors.fill: parent
    }
}
