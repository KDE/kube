/*
 * Copyright (C) 2015 Michael Bohlender <michael.bohlender@kdemail.net>
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
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Item {
    id: root

    property StackView stack;
    property string mailId;

    //toolbar
    ToolBar {
        id: toolBar

        RowLayout {

            width: parent.width

            spacing: unit.width * 8

            ToolButton {
                iconName: "go-previous"

                onClicked: stack.pop()
            }

            //FIXME
            Row {
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: unit.width * 8

                ToolButton {
                    iconName: "mail-mark-unread-new"
                }
                ToolButton {
                    iconName: "mail-mark-important"
                }
                ToolButton {
                    iconName: "user-trash"
                }
            }

            ToolButton {

                anchors.right: parent.right

                iconName: "mail-reply-sender"

                onClicked: stack.push({"item": Qt.resolvedUrl("ComposerView.qml"), properties: {stack: stack}})
            }
        }
    }

    //main content
    Item {

        anchors {
            top: toolBar.bottom
            right: parent.right
            left: parent.left
            bottom: parent.bottom
        }

        Item {
            id: model
            property string subject: "We need more Food"
            property string sender: "Alice Trump"
            property string senderAddress: "alice@wonderland.net"
            property string cc: "vdg@kde.org; ross@ccmail.com"
            property string time: "2 days ago"
            property string body: "Hi Bob, \n \n Ut nibh massa, volutpat quis diam quis, tincidunt consectetur massa. Nulla eu ultricies justo, eu aliquam lacus. Maecenas at interdum est, at luctus nibh. Quisque scelerisque consequat lectus vitae egestas. Maecenas molestie risus id enim consequat dapibus. Ut dapibus hendrerit est, ut aliquam ex fringilla commodo. Donec rutrum consectetur dapibus. Fusce hendrerit pulvinar lacinia. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent semper sit amet elit ut volutpat. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae. \n \n Cheers, \n \n Alice"
        }

        Rectangle {
            id: background

            anchors.fill: parent

            color: colorPalette.background
        }

        Item {
            anchors {
                fill: parent
                margins: unit.width * 10
            }

            Column {
                id: content

                width: parent.width

                spacing: unit.width * 5


                Label {
                    text: model.subject

                    font.pixelSize: 22 // Fixme
                }

                //FIXME use propper avatar or avatar replacement
                //Rectangle {

                Item {
                    height: avatar.height
                    width: parent.width

                    Avatar {
                        id: avatar

                        width: unit.width * 15
                        height: unit.width * 15

                        name: model.sender
                    }

                    Label {

                        anchors {
                            left: avatar.right
                            top: avatar.top
                            leftMargin: unit.width * 3
                        }

                        text: model.senderAddress
                    }

                    Label {
                        anchors {
                            left: avatar.right
                            bottom: avatar.bottom
                            leftMargin: unit.width * 3
                        }

                        text: "CC: " + model.cc

                    }
                }

                Label {

                    width: parent.width

                    wrapMode: Text.WordWrap

                    text: model.body

                }
            }
        }
    }
}
