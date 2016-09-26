/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import org.kde.kirigami 1.0 as Kirigami

import QtQml 2.2 as QtQml

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.theme 1.0

Item {
    id: root

    property variant mail;

    ScrollView {

        anchors.fill: parent

        ListView {
            anchors.fill: parent

            model: KubeFramework.MailListModel {
                mail: root.mail
            }

            header: Item {
                height: Kirigami.Units.gridUnit
                width: parent.width

            }

            footer: Item {
                 height: Kirigami.Units.gridUnit * 2
                 width: parent.width
            }

            delegate: Item {

                height: sheet.height + Kirigami.Units.gridUnit * 2
                width: parent.width

                Rectangle {
                    id: sheet
                    anchors.centerIn: parent
                    implicitHeight: header.height + body.height + (Kirigami.Units.gridUnit * 2.5) * 2 + footer.height
                    width: parent.width - Kirigami.Units.gridUnit * 4

                    //TODO bookmark
                    /*
                    ToolButton {
                        iconName: "bookmark-new"

                    }
                    */
                    Item {
                        id: header

                        height: Kirigami.Units.gridUnit * 5
                        width: parent.width

                        Row {
                            id: headerContent
                            anchors {
                                left: seperator.left
                                bottom: seperator.top
                                bottomMargin: height * 0.25
                            }

                            spacing: Kirigami.Units.largeSpacing / 2

                            Avatar {
                                id: avatar

                                height: Kirigami.Units.gridUnit * 2.5
                                width: height

                                name: model.sender
                            }

                            ColumnLayout {

                                RowLayout {

                                    Text {
                                        text: model.senderName

                                        font.weight: Font.DemiBold
                                        color: Kirigami.Theme.textColor
                                        opacity: 0.75
                                    }

                                    //TODO not yet in model
                                    /*
                                    Text {
                                        text: model.senderAd

                                        color: Kirigami.Theme.textColor
                                        opacity: 0.75
                                    }
                                    */
                                }

                                RowLayout {
                                    Kirigami.Label {
                                        text: "To:"
                                    }
                                    Text {
                                        text: "TODO TODO TODO"//model.receivers TODO not yet in model

                                        color: Kirigami.Theme.textColor
                                        opacity: 0.75
                                    }
                                }
                            }
                        }

                        Text {

                            anchors {
                                right: seperator.right
                                bottom: headerContent.top
                            }

                            text: Qt.formatDateTime(model.date)

                            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.7
                            color: Kirigami.Theme.textColor
                            opacity: 0.75
                        }

                        Rectangle {
                            id: seperator

                            anchors {
                                bottom: parent.bottom
                                horizontalCenter: parent.horizontalCenter
                            }

                            width: parent.width - Kirigami.Units.gridUnit * 2
                            height: 1

                            color: Kirigami.Theme.textColor
                            opacity: 0.5
                        }

                        Rectangle {
                            anchors {
                                bottom: seperator.top
                                right: seperator.right
                            }

                            height: Kirigami.Units.gridUnit
                            width: height

                            color: Kirigami.Theme.textColor
                            opacity: 0.5
                        }
                    }

                    MailViewer {
                        id: body

                        anchors {
                            top: header.bottom
                            left: parent.left
                            right: parent.right
                            leftMargin: avatar.height + Kirigami.Units.gridUnit
                            rightMargin: avatar.height + Kirigami.Units.gridUnit
                            topMargin: avatar.height
                        }

                        width: header.width - Kirigami.Units.largeSpacing * 2
                        height: desiredHeight

                        message: model.mimeMessage
                    }

                    Item {
                        id: footer

                        anchors.bottom: parent.bottom

                        height: Kirigami.Units.gridUnit * 3
                        width: parent.width

                        Text {

                            anchors{
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                leftMargin: Kirigami.Units.gridUnit
                            }

                            text: "Delete Mail"
                            color: Kirigami.Theme.textColor
                            opacity: 0.5
                        }

                        ToolButton {
                            anchors{
                                verticalCenter: parent.verticalCenter
                                right: parent.right
                                rightMargin: Kirigami.Units.gridUnit
                            }

                            iconName: "mail-reply-sender"
                        }
                    }

                }
            }
        }
    }
}
