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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.2

import org.kde.kirigami 1.0 as Kirigami

Controls.ScrollView {
    id: root

    anchors.fill: parent

    ListView {
        id: listView

        model: MailListModel {}

        delegate: Kirigami.AbstractListItem {
            id: mailListDelegate

            width: listView.width
            //height: Kirigami.Units.gridUnit * 3.5

            supportsMouseEvents: true
            enabled: true


            Item {
                width: parent.width
                height: Kirigami.Units.gridUnit * 2.85

                RowLayout {

                    Item {
                        width: 1
                    }

                    Avatar {
                        id: avatar

                        height: Kirigami.Units.gridUnit * 2.7
                        width: height

                        name: model.sender
                    }

                    Item {
                        width: 1
                    }

                    Column {
                        id: textItem

                        Text {
                            id: subjectLabel

                            text: model.subject

                            color: important ? "#da4453" : unread ? "#1d99f3" : Kirigami.Theme.textColor

                            font.weight: model.unread || model.important ? Font.DemiBold : Font.Normal
                        }

                        Text {
                            text: model.sender

                            font.pointSize: Kirigami.Units.gridUnit * 0.4

                        }

                        Text {
                            text: model.date

                            font.pixelSize: Kirigami.Units.gridUnit * 0.6

                            opacity: 0.5
                        }
                    }
                }

                Rectangle {

                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                        bottomMargin: 5
                        //verticalCenter: parent.verticalCenter
                    }

                    color: "lightgrey"//Kirigami.Theme.complementaryBackgroundColor

                    height: Kirigami.Units.iconSizes.medium
                    width: height

                    visible: unread ? true : false

                    radius: 100

                    Text {
                        anchors.centerIn: parent
                        text: "+4"
                        color: Kirigami.Theme.complementaryTextColor
                        font.weight: Font.DemiBold
                    }
                }

            }
        }
    }
}
