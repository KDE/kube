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

    property StackView stack

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

            ToolButton {
                iconName: "mail-reply-sender"
            }

            //FIXME spacer?
            Label {
                text: ""
                Layout.fillWidth: true
            }

            ToolButton {
                iconName: "mail-attachment"
            }

            ToolButton {
                iconName: "mail-send"
            }
        }
    }

    //main content
    Rectangle {

        anchors {
            top: toolBar.bottom
            right: parent.right
            left: parent.left
            bottom: parent.bottom
        }

        color: colorPalette.background

        ColumnLayout {

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right

                margins: unit.width * 5

            }

            spacing: unit.height

            RowLayout {
                id: from

                width: parent.width

                spacing:  unit.width * 5

                Label {
                    text: "From"
                }

                Button {
                    Layout.fillWidth: true

                    text: "meep@monkey.com"

                }

            }

            RowLayout {
                id: to

                width: parent.width

                spacing:  unit.width * 5

                Label {
                    text: "To"
                }

                TextField {
                    Layout.fillWidth: true

                }

            }
        }
    }
}
