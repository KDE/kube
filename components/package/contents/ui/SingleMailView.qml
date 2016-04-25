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
import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.theme 1.0

Rectangle {
    id: root
    property variant mail;

    color: "grey"

    ScrollView {
        anchors.fill: parent

    ListView {
        anchors.verticalCenter: parent.verticalCenter

        width: parent.width

        header: Item {
            height: Kirigami.Units.largeSpacing
        }

        footer: Item {
            height: Kirigami.Units.largeSpacing * 3
        }

        model: KubeFramework.MailListModel {
            mail: root.mail
        }

        delegate: Item {

            width: root.width
            implicitHeight: content.height

            Rectangle {
                id: content
                anchors.centerIn: parent

                width: parent.width * 0.9
                implicitHeight: header.height + body.height + footer.height + Kirigami.Units.gridUnit * 8

                    Item {
                        id: header

                        anchors {
                            top: parent.top
                            topMargin: Kirigami.Units.largeSpacing
                            horizontalCenter: parent.horizontalCenter
                        }

                        width: parent.width - Kirigami.Units.largeSpacing * 2
                        height: Kirigami.Units.gridUnit * 6


                        Avatar  {
                            id: avatar

                            height: Kirigami.Units.gridUnit * 4
                            width: height

                            name: model.senderName
                        }

                        Text {

                            anchors {
                                bottom: parent.bottom
                                left: parent.left
                                bottomMargin: Kirigami.Units.smallSpacing
                            }

                            text: model.subject
                            renderType: Text.NativeRendering
                            color: Kirigami.Theme.textColor
                        }

                        Text {

                            anchors {
                                top: avatar.top
                                left: avatar.right
                                leftMargin: Kirigami.Units.smallSpacing
                            }

                            text: model.senderName

                            renderType: Text.NativeRendering
                            color: Kirigami.Theme.textColor
                        }

                        Text {

                            anchors {
                                right: parent.right
                            }
                            text: Qt.formatDateTime(model.date)

                            renderType: Text.NativeRendering
                            color: Kirigami.Theme.textColor
                        }

                        Rectangle {

                            anchors {
                                bottom: border.top
                                right: border.right
                            }

                            height: Kirigami.Units.iconSizes.small
                            width: height

                            color: Kirigami.Theme.complementaryBackgroundColor
                            opacity: 0.5
                        }

                        Rectangle {
                            id: border

                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1

                            color: Kirigami.Theme.complementaryBackgroundColor

                            opacity: 0.5
                        }
                    }

                    Text {
                        id: body

                        anchors {
                            top: header.bottom
                            topMargin: Kirigami.Units.largeSpacing * 2
                            left: header.left
                            leftMargin: Kirigami.Units.largeSpacing
                        }

                        width: header.width - Kirigami.Units.largeSpacing * 2

                        text: model.mimeMessage

                        clip: true
                        wrapMode: Text.WordWrap
                        renderType: Text.NativeRendering
                        color: Kirigami.Theme.textColor
                    }

                    /*
                    MailViewer {
                        message: model.mimeMessage
                    }
                    */

                    Item {
                        id: footer

                        anchors {
                            bottom: parent.bottom
                            bottomMargin: Kirigami.Units.largeSpacing
                            horizontalCenter: parent.horizontalCenter
                        }

                        width: header.width
                        height: Kirigami.Units.gridUnit

                        ToolButton {
                            text: "Delete Email"
                        }
                    }
                }
            }
        }
    }
}
