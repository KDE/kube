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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import org.kde.kirigami 1.0 as Kirigami

Rectangle {
    id: root

    function loadMessage(message, loadAsDraft) {
        composer.loadMessage(message, loadAsDraft);
    }

    color: Kirigami.Theme.textColor

    opacity: 0.9

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.destroy()
        }
    }

    Rectangle {
        anchors.centerIn: parent

        height: root.height * 0.8
        width: root.width * 0.8

        color: Kirigami.Theme.backgroundColor

        MouseArea {
            anchors.fill: parent
        }

        ColumnLayout {

            anchors {
                fill: parent
                margins: Kirigami.Units.largeSpacing
            }

            Composer {
                id: composer

                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight

                Button {
                    text: "Send"

                    onClicked: {
                        composer.send()
                        root.destroy()
                    }
                }
            }
        }
    }
}
