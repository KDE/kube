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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3
import Qt.labs.calendar 1.0

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root

    DayOfWeekRow {
        anchors {
            left: monthGrid.left
            bottom: monthGrid.top
        }
        width: monthGrid.width

        delegate: Kube.Label {
            text: model.shortName
        }
    }

    MonthGrid {
        id: monthGrid

        anchors.centerIn: parent

        spacing: 0

        delegate: Rectangle {
            width: Kube.Units.gridUnit * 7
            height: Kube.Units.gridUnit * 5

            color: Kube.Colors.viewBackgroundColor
            border.color: Kube.Colors.buttonColor
            border.width: 1

            Item {
                id: dayInfo

                height: Kube.Units.gridUnit
                width: parent.width

                Kube.Label {
                    anchors {
                        top: parent.top
                        left: parent.left
                        topMargin: Kube.Units.smallSpacing
                        leftMargin: Kube.Units.smallSpacing
                    }
                    text: model.day
                }
            }

            ListView {
                anchors {
                    top: dayInfo.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    topMargin: Kube.Units.smallSpacing
                    leftMargin: 1
                    rightMargin: 1
                }
                clip: true

                model: 2

                delegate: Rectangle {

                    width: parent.width - 2
                    height: Kube.Units.gridUnit
                    color: "blue"
                    border.width: 1
                    border.color: Kube.Colors.viewBackgroundColor
                }
            }
        }
    }
}
