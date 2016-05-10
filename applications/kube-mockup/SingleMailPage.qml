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
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

import org.kde.kirigami 1.0 as Kirigami
Item {
    id: conversations

    ScrollView {

        anchors.fill: parent

        ListView {
            anchors.fill: parent

            model: ListModel {
                ListElement {
                    sender: "Sender McSenderson sender@sentmail.com"
                    senderName: "Sender McSenderson"
                    senderAd: "<sender@sentmail.com>"
                    receivers: "Me, Van Receiverson, Alice A."
                    subject: "I feel Week without my bananas"
                    date: "Yesterday 20:37"
                    text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean mattis nisi purus, sit amet hendrerit erat ornare et. Praesent eu risus iaculis, sollicitudin turpis vitae, cursus elit. Sed vehicula, lectus in blandit sagittis, lacus tortor finibus dui, blandit aliquet felis purus quis neque. Nunc luctus tempor venenatis. Phasellus lobortis vel sapien vitae tempus. Morbi mi diam, dictum vitae placerat vel, hendrerit quis elit. Mauris ultricies sit amet massa at aliquet. Sed ac vulputate velit. Duis malesuada quam eget nunc sollicitudin, id sollicitudin diam egestas. Integer maximus facilisis ipsum sed egestas. Donec scelerisque, felis id tincidunt tempus, sem mauris malesuada lectus, nec aliquet mauris lorem fermentum nibh. Vestibulum aliquam dui mi, eget pulvinar orci tempor eget. In dui erat, pharetra vitae lorem vitae, congue sollicitudin nunc. Maecenas sit amet metus ac neque laoreet tempor. Pellentesque euismod congue lacinia. Duis sagittis arcu ac felis fringilla, ac tempor leo convallis."
                }
                ListElement {
                    sender: "Sender McSenderson sender@sentmail.com"
                    senderName: "Sender McSenderson"
                    senderAd: "<sender@sentmail.com>"
                    receivers: "Me, Van Receiverson, Alice A."
                    subject: "I feel Week without my bananas"
                    date: "Yesterday 20:37"
                    text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean mattis nisi purus, sit amet hendrerit erat ornare et. Praesent eu risus iaculis, sollicitudin turpis vitae, cursus ."
                }
            }

            header: Item {
                height: Kirigami.Units.gridUnit * 3
                width: parent.width

                 Kirigami.Heading {

                                     anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.gridUnit * 2
                    bottom: parent.bottom
                }

                    text: "I feel Week without my bananas" + ":"

                    level: 2
                 }
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
                    width: parent.width * 0.9

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

                                    Text {
                                        text: model.senderAd

                                        color: Kirigami.Theme.textColor
                                        opacity: 0.75
                                    }
                                }

                                RowLayout {
                                    Kirigami.Label {
                                        text: "To:"
                                    }
                                Text {
                                    text: model.receivers

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

                            text: model.date

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

                    Item {
                        id: body
                        anchors {
                            top: header.bottom
                            left: parent.left
                            right: parent.right
                            leftMargin: avatar.height + Kirigami.Units.gridUnit
                            rightMargin: avatar.height + Kirigami.Units.gridUnit
                            topMargin: avatar.height
                        }

                        implicitHeight: textContent.height

                        Text {
                            id: textContent
                            width: parent.width

                            text: model.text
                            wrapMode: Text.WordWrap
                            color: Kirigami.Theme.textColor
                        }

                    }

                    Rectangle {
                        id: footer

                        anchors.bottom: parent.bottom

                        height: Kirigami.Units.gridUnit * 3
                        width: parent.width

                        //color:"lightgrey"

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


            /*

            Item {

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

                        text: model.text
                    }

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

            /*
            Item {


                    width: parent.width
                    height: mail.height + 400

                    Rectangle {
                        id: mail

                        anchors {
                            centerIn: parent
                        }

                        height: header.height + content.height
                        width: parent.width * 0.9

                        Item {
                            id: header

                            anchors {
                                top: parent.top
                                left: parent.left
                                right: parent.right
                                margins: Kirigami.Units.largeSpacing
                            }

                            height: Kirigami.Units.gridUnit * 3

                            Avatar {
                                name: model.sender

                                height: Kirigami.Units.iconSizes.huge
                                width: height
                            }

                            Rectangle {
                                anchors {
                                    bottom: parent.bottom
                                    left: parent.left
                                    right: parent.right
                                }

                                height: 1

                                color: "lightgrey"
                            }

                        }

                        Text {
                            id: content

                            anchors {
                                top: header.bottom
                                left: header.left
                                right: header.right
                                topMargin: Kirigami.Units.smallSpacing
                            }

                            width: header.width

                            text: model.text

                            clip: true

                            wrapMode: Text.WordWrap

                        }

                    }
            }

        }
    }
}
   */
