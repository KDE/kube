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
    property string folderId;

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

            Label {
                Layout.fillWidth: true

                text: "Current Folder Name"
            }

            ToolButton {
                anchors.right: parent.right

                iconName: "system-search"
            }
        }
    }

    //main content
    ListView {
        id: listView

        anchors {
            top: toolBar.bottom
            right: parent.right
            left: parent.left
            bottom: parent.bottom
        }

        clip: true

        model: MailListModel { }

        delegate:  Item {
            id: delegateRoot

            readonly property bool isCurrentItem: ListView.isCurrentItem

            height: unit.width * 25
            width: parent.width

            MouseArea {
                id: mouseArea

                anchors.fill: parent

                onClicked:  {
                    stack.push({"item": Qt.resolvedUrl("SingleMailView.qml"), properties: {stack: stack}})
                }
            }

            Rectangle {
                anchors.fill: parent

                color: colorPalette.background


                //read
                Rectangle {
                    anchors.fill: parent

                    color: colorPalette.read
                    opacity: 0.1

                    visible: delegateRoot.isCurrentItem !== model.index && model.unread === false
                }

                //clickColor
                Rectangle {
                    id: clickColor
                    anchors.fill: parent

                    color: colorPalette.selected
                    opacity: 0.4

                    visible: mouseArea.pressed
                }

                //border
                Rectangle {
                    anchors {
                        bottom: parent.bottom
                    }

                    height: 1
                    width: parent.width

                    color: "#232629" //FIXME themeable?
                    opacity: 0.2
                }
            }


            Avatar {
                id: avatar
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: unit.width * 4
                }

                width: unit.width * 15
                height: unit.width * 15

                name: model.senderName

            }

            Label {
                id: senderName

                anchors {
                    top: avatar.top
                    left: avatar.right
                    right: parent.right
                    leftMargin: unit.width * 2
                    rightMargin: unit.width * 2
                }

                text: model.senderName

                font.pointSize: 12 //FIXME ?
            }

            Label {
                id: subject

                anchors {
                    top: senderName.bottom
                    left: senderName.left
                    right: parent.right

                    topMargin: unit.width * 2
                    rightMargin: unit.width * 2
                }

                text: model.subject

                font.weight: Font.DemiBold
            }

            Label {
                id: date

                anchors {
                    top: avatar.top
                    right: parent.right
                    rightMargin: unit.width * 2
                }

                text: model.date

                font.weight: Font.Light
                font.italic: true
            }
        }
    }
}
