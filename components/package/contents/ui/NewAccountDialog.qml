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

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.theme 1.0

Rectangle {
    id: root

    color: ColorPalette.border

    opacity: 0.9

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.destroy()
        }
    }

    Rectangle {
        id: dialog
        anchors.centerIn: parent

        height: root.height * 0.8
        width: root.width * 0.8

        color: ColorPalette.background

        MouseArea {
            anchors.fill: parent
        }

        ToolBar {
            id: toolBar
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            ToolButton {

                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }

                iconName: "go-previous"

                onClicked: stack.pop()
            }
        }

        StackView {
            id: stack

            anchors {
                top: toolBar.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            clip: true
            initialItem: accountTypes

            Component {
                id: accountTypes

                Item {
                    id: pageRoot

                    height: dialog.height
                    width: dialog.width

                    Column {
                        anchors.centerIn: parent

                        spacing: Kirigami.Units.largeSpacing

                        Button {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: pageRoot.width * 0.4

                            text: "kolabnow"

                            onClicked: {
                                stack.push(kolabnow)
                            }
                        }

                        Button {
                            anchors.horizontalCenter: parent.horizontalCenter
                             width: pageRoot.width * 0.4

                            text: "imap"

                            onClicked: {
                                stack.push(test)
                            }
                        }

                        Button {
                            anchors.horizontalCenter: parent.horizontalCenter
                             width: pageRoot.width * 0.4

                            text: "maildir"
                        }
                    }
                }
            }

            Component {
                id: test

                Rectangle {
                    color: "green"

                    height: dialog.height
                    width: dialog.width
                }
            }

            Component {
                id: kolabnow

                Item {
                    id: pageRoot

                    height: dialog.height
                    width: dialog.width

                    Column {
                        anchors.centerIn: parent

                        spacing: Kirigami.Units.largeSpacing

                        TextField {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: pageRoot.width * 0.4

                            placeholderText: "Title of Account (e.g. work, private, kolabnow...)"
                        }

                        TextField {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: pageRoot.width * 0.4

                            placeholderText: "Email (e.g. jens.moep@kolabnow.com)"
                        }

                        TextField {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: pageRoot.width * 0.4

                            placeholderText: "Password"
                        }
                    }
                }
            }
        }
    }
}
