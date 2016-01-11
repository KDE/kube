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

    //toolbar
    ToolBar {
        id: toolBar

        RowLayout {

            width: parent.width

            spacing: unit.width * 5

            ToolButton {
                anchors.right: parent.right

                iconName: "system-search"
            }
        }
    }

    //main content
    ListView {

        anchors {
            top: toolBar.bottom
            right: parent.right
            left: parent.left
            bottom: parent.bottom
        }

        clip: true

        model: FolderListModel { }


        delegate: Item {

            height: unit.width * 20
            width: parent.width

            MouseArea {
                id: mouseArea

                anchors.fill: parent

                onClicked: {
                    stack.push({"item": Qt.resolvedUrl("MailListView.qml"), properties: {stack: stack, folderId: model.name}})
                }
            }

            //background
            Rectangle {
                anchors.fill: parent

                color: colorPalette.background
            }

            //clickColor
            Rectangle {
                id: clickColor
                anchors.fill: parent

                color: colorPalette.selected
                opacity: 0.4

                visible: mouseArea.pressed
            }

            //FIXME without useing PlasmaComponents
            ToolButton {
                id: icon

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: unit.width * 10
                }

                iconName: model.icon

            }

            Label {

                anchors{
                    left: icon.right
                    leftMargin: unit.width * 15
                    verticalCenter: icon.verticalCenter
                }

                text: model.name

                font.weight: model.topLvl ? Font.DemiBold : Font.Normal
            }
        }
    }
}
