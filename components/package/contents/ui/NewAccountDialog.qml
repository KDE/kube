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
import org.kube.components 1.0 as KubeComponents

KubeComponents.OverlayDialog {
    id: root

    KubeFramework.AccountsController {
        id: accountsController
    }

    Item {
        id: dialog

        anchors.centerIn: parent

        height: parent.height * 0.8
        width: parent.width * 0.8

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

                visible: stack.depth > 1
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
                                accountsController.createAccount("imap");
                                root.closeDialog()
                                //stack.push(imap)
                            }
                        }

                        Button {
                            anchors.horizontalCenter: parent.horizontalCenter
                             width: pageRoot.width * 0.4

                            text: "maildir"

                            onClicked: {
                                accountsController.createAccount("maildir");
                                root.closeDialog()
                                //stack.push(maildir)
                            }
                        }
                    }
                }
            }

            Component {
                id: imap

                Rectangle {
                    color: "green"

                    height: dialog.height
                    width: dialog.width
                }
            }

            Component {
                id: maildir

                Rectangle {
                    color: "blue"

                    height: dialog.height
                    width: dialog.width
                }
            }

            Component {
                id: kolabnow

                Rectangle {
                    color: "yellow"

                    height: dialog.height
                    width: dialog.width
                }
            }

        }
    }
}
