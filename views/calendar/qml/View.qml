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


    Item {
        anchors {
            top: parent.top
            topMargin: Kube.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }

        width: Kube.Units.gridUnit * 7 * 7 + Kube.Units.gridUnit * 2
        height: Kube.Units.gridUnit * 27

        DayOfWeekRow {
            id: dayLabels
            anchors.right: parent.right
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
            id: daylong

            anchors {
                top: dayLabels.bottom
                right: parent.right
            }

            height: Kube.Units.gridUnit * 3
            width: Kube.Units.gridUnit * 7 * 7
            color: Kube.Colors.viewBackgroundColor
            border.width: 1
            border.color: Kube.Colors.buttonColor

            ListView {

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

        Flickable {
            id: mainWeekViewer

            anchors {
                top: daylong.bottom
            }

            height: Kube.Units.gridUnit * 24
            width: Kube.Units.gridUnit * 7 * 7 + Kube.Units.gridUnit * 2

            contentHeight: Kube.Units.gridUnit * 24 * 2
            contentWidth: width

            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Kube.ScrollBar {}

            Row {

                height: Kube.Units.gridUnit * 24 * 2
                width: Kube.Units.gridUnit * 7 * 7 + Kube.Units.gridUnit * 2

                spacing: 0

                Column {
                    anchors.bottom: parent.bottom
                    Repeater {
                        model: ["0:00","1:00","2:00","3:00","4:00","5:00","6:00","7:00","8:00","9:00","10:00","11:00","12:00","13:00","14:00","15:00","16:00","17:00","18:00","19:00","20:00","21:00","22:00","23:00"]
                        delegate: Item {
                            height: Kube.Units.gridUnit * 2
                            width: Kube.Units.gridUnit * 2

                            Kube.Label {
                                anchors {
                                    right: parent.right
                                    rightMargin: Kube.Units.smallSpacing
                                }
                                text: model.modelData
                            }
                        }
                    }
                }

                Repeater {
                    model: WeekEvents{}
                    delegate: Rectangle {
                        id: day

                        property var events: model.events

                        width: Kube.Units.gridUnit * 7
                        height: Kube.Units.gridUnit * 24 * 2

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
}
