/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.9
import QtQuick.Controls 1.3 as Controls1
import QtQuick.Layouts 1.2

import org.kube.framework 1.0 as Kube


RowLayout {
    id: root

    //TODO update every second
    property date currentDate: new Date()

    anchors.fill: parent

    Rectangle {
        width: Kube.Units.gridUnit * 10
        Layout.fillHeight: parent.height
        color: Kube.Colors.darkBackgroundColor

        Kube.PositiveButton {
            id: newEventButton
            objectName: "newEventButton"

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: Kube.Units.largeSpacing
            }
            focus: true
            text: qsTr("New Event")
            onClicked: {}
        }

        Column {
            anchors {
                top: newEventButton.bottom
                left: newEventButton.left
                topMargin: Kube.Units.largeSpacing
            }

            width: parent.width
            spacing: Kube.Units.smallSpacing

            Kube.Label {
                text: "Week"
                color: Kube.Colors.highlightedTextColor
            }

            Kube.Label {
                text: "Month"
                color: Kube.Colors.highlightedTextColor
            }

            Kube.Label {
                text: "Agenda"
                color: Kube.Colors.highlightedTextColor
            }
        }

        Column {
            anchors {
                bottom: parent.bottom
                left: newEventButton.left
                bottomMargin: Kube.Units.largeSpacing
            }

            spacing: Kube.Units.smallSpacing

            Repeater {
                model: ["calendar_1","calendar_2","calendar_3"]
                delegate: Row {
                    spacing: Kube.Units.smallSpacing
                    Kube.CheckBox {
                        opacity: 0.9
                    }
                    Kube.Label {
                        text: modelData
                        color: Kube.Colors.highlightedTextColor
                    }
                }
            }
        }
    }

    WeekView {
        Layout.fillHeight: true
        Layout.fillWidth: true
        currentDate: root.currentDate
    }
}
