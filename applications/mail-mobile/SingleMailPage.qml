/*
 * Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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
import QtQuick.Controls 1.3 as Controls
import QtQuick.Layouts 1.1

import org.kde.kirigami 1.0 as Kirigami

Kirigami.ScrollablePage {
    id: root

    background: Rectangle {
        color: Kirigami.Theme.viewBackgroundColor
    }

    title: "Read"

    mainAction: Kirigami.Action {
        iconName: "mail-reply-sender"

        onTriggered: {
            app.pageStack.push(Qt.resolvedUrl("ComposerPage.qml"))
        }

    }
    contextualActions: [
        Kirigami.Action {
            text:"Forward"
            iconName: "mail-forward"
            onTriggered: {
                app.contextDrawer.close()
                app.pageStack.push(Qt.resolvedUrl("ComposerPage.qml"))
            }
        }
    ]

    Item {

        Layout.fillWidth: true

        Item {
                id: model
                property string subject: "We need more Food"
                property string sender: "Alice Trump"
                property string senderAddress: "alice@wonderland.net"
                property string cc: "vdg@kde.org; ross@ccmail.com"
                property string time: "2 days ago"
                property string body: "Hi Bob, \n \n Ut nibh massa, volutpat quis diam quis, tincidunt consectetur massa. Nulla eu ultricies justo, eu aliquam lacus. Maecenas at interdum est, at luctus nibh. Quisque scelerisque consequat lectus vitae egestas. Maecenas molestie risus id enim consequat dapibus. Ut dapibus hendrerit est, ut aliquam ex fringilla commodo. Donec rutru semper sit amet elit ut volutpat. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae. \n \n Cheers, \n \n Alice"
        }

        Item {
            id: content

            width: parent.width

            Kirigami.Heading {
                id: subject
                text: model.subject

                Layout.fillWidth: true
            }

            Kirigami.Label {

                anchors {
                    top: subject.bottom
                }

                text: "From:"
            }

            Kirigami.Label {
                id: sender
                anchors {
                    top: avatar.top
                    right: avatar.left
                }

                text: model.sender
            }

            Kirigami.Label {

                anchors {
                    bottom: avatar.bottom
                    right: avatar.left
                }

                text: model.senderAddress
            }

            Avatar {
                id: avatar

                anchors {
                    top: subject.bottom
                    right: content.right
                }
                name: model.sender

                height: subject.height
                width: subject.height
            }

            Kirigami.Label {
                anchors.top: avatar.bottom

                width: parent.width

                wrapMode: Text.WordWrap

                text: model.body
            }
        }
    }
}
