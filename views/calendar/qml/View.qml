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
import QtQuick.Controls 2.0
import Qt.labs.calendar 1.0

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root

    property var month: Calendar.March
    property var year: 2017

    Column {
        anchors.centerIn: parent

        DayOfWeekRow {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0
            locale: Qt.locale("de")

            delegate: Rectangle {
                width: Kube.Units.gridUnit * 7
                height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 3

                border.width: 1
                border.color: "lightgrey"
                color: Kube.Colors.viewBackgroundColor

                Kube.Label {
                    anchors {
                        top: parent.top
                        left: parent.left
                        margins: Kube.Units.smallSpacing
                    }
                    text: model.shortName
                }
            }
        }

        Rectangle {
            height: Kube.Units.gridUnit * 3
            width: parent.width
            color: Kube.Colors.viewBackgroundColor
            border.width: 1
            border.color: Kube.Colors.buttonColor

            ListView {
                id: daylong

                anchors {
                    fill: parent
                    margins: 1
                }

                model: DaylongEvents {}

                delegate: Item {
                    height: Kube.Units.gridUnit + 2 // +2 to make good for the white border
                    width: daylong.width

                    Rectangle {
                        width: Kube.Units.gridUnit * 7 * model.duration
                        height: parent.height
                        x: Kube.Units.gridUnit * 7 * model.starts
                        color: model.color
                        border.width: 1
                        border.color: Kube.Colors.viewBackgroundColor

                        Kube.Label {
                            anchors {
                                left: parent.left
                                leftMargin: Kube.Units.smallSpacing
                            }
                            color: Kube.Colors.highlightedTextColor
                            text: model.text
                        }
                    }
                }
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 0

            Repeater {
                model: WeekEvents{}
                delegate: Rectangle {
                    id: day

                    property var events: model.events

                    width: Kube.Units.gridUnit * 7
                    height: Kube.Units.gridUnit * 20

                    border.width: 1
                    border.color: "lightgrey"
                    color: Kube.Colors.viewBackgroundColor

                    Repeater {
                        model: parent.events

                        delegate: Rectangle {
                            anchors {
                                right: parent.right
                                rightMargin: Kube.Units.smallSpacing
                            }
                            width: parent.width - Kube.Units.smallSpacing * 2 - Kube.Units.gridUnit * model.indention
                            height: Kube.Units.gridUnit * model.duration
                            y: Kube.Units.gridUnit * model.starts
                            x: Kube.Units.gridUnit * model.indention

                            color: model.color
                            border.width: 1
                            border.color: Kube.Colors.viewBackgroundColor

                            Kube.Label {
                                anchors {
                                    left: parent.left
                                    leftMargin: Kube.Units.smallSpacing
                                }
                                text: model.text
                                color: Kube.Colors.highlightedTextColor
                            }
                        }
                    }
                }
            }
        }
    }
}
