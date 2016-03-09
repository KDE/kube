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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import QtQml 2.2

import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kube.framework.domain 1.0 as Mail

ScrollView {
    id: root
    property variant parentFolder
    property variant currentMail

    ListView {
        id: listView

        model: Mail.MailListModel {
            parentFolder: root.parentFolder
        }

        Keys.onDownPressed: {
            incrementCurrentIndex()
        }
        Keys.onUpPressed: {
            decrementCurrentIndex()
        }
        focus: true

        delegate: PlasmaComponents.ListItem {
            width: listView.width
            height: unit.size * 12

            enabled: true
            checked: listView.currentIndex == index

            Binding {
                target: root
                property: "currentMail"
                when: listView.currentIndex == index
                value: model.domainObject
            }

            MouseArea {
                anchors.fill: parent

                onClicked:  {
                    listView.currentIndex = model.index
                }
            }

            Rectangle {
                id: unread

                anchors.fill: parent

                // color: colorPalette.read
                color: "steelblue"
                opacity: 0.1

                visible: model.unread
            }

            Avatar {
                id: avatar

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: unit.size * 2
                }

                height: unit.size * 9
                width: height

                name: model.senderName
            }

            Label {
                id: senderName

                anchors {
                    top: avatar.top
                    left: avatar.right
                    leftMargin: unit.size * 3
                }

                text: model.senderName

                font.weight: Font.DemiBold
            }

            //The name currently contains both name and address
            // Label {
            //     id: sender
            //
            //     anchors {
            //         top: avatar.top
            //         left: senderName.right
            //         leftMargin: unit.size
            //         right: date.left
            //         rightMargin: unit.size
            //     }
            //
            //     text: "(" + model.sender +")"
            //
            //     clip: true
            // }

            Label {
                id: date

                anchors {
                    top: avatar.top
                    right: parent.right
                    rightMargin: unit.size * 2
                }

                text: Qt.formatDateTime(model.date)

                font.weight: Font.Light
            }

            Label {
                id: subject

                anchors {
                    bottom: avatar.bottom
                    left: avatar.right
                    leftMargin: unit.size * 3
                }

                text: model.subject
            }
        }
    }
}
