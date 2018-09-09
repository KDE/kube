/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
 *  Copyright (C) 2018 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import QtQuick.Controls 2.2

import org.kube.framework 1.0 as Kube
import "dateutils.js" as DateUtils

Item {
    id: root
    property int daysToShow
    property int daysPerRow: daysToShow
    property var dayWidth
    property date currentDate
    property date startDate
    property var calendarFilter
    property bool paintGrid: false
    property bool showDayIndicator: false
    property var filter
    property alias dayHeaderDelegate: dayLabels.delegate
    property Component weekHeaderDelegate

    //Internal
    property int numberOfLinesShown: 0
    property int numberOfRows: (daysToShow / daysPerRow)
    property var dayHeight: height / numberOfRows

    width: root.dayWidth * root.daysPerRow

    implicitHeight: (numberOfRows > 1 ? Kube.Units.gridUnit * 10 * numberOfRows: numberOfLinesShown * Kube.Units.gridUnit) + dayLabels.height

    height: implicitHeight
    visible: numberOfRows > 1 || numberOfLinesShown

    Column {
        anchors {
            fill: parent
        }

        DayLabels {
            id: dayLabels
            startDate: root.startDate
            dayWidth: root.dayWidth
            daysToShow: root.daysPerRow
        }

        //Weeks
        Repeater {
            model: Kube.MultiDayEventModel {
                model: Kube.EventModel {
                    start: root.startDate
                    length: root.daysToShow
                    calendarFilter: root.calendarFilter
                    filter: root.filter ? root.filter : {}
                }
                // daysPerRow: root.daysPerRow //Hardcoded to 7
            }
            //One row => one week
            Row {
                width: parent.width
                Loader {
                    id: weekHeader
                    height: root.dayHeight
                    sourceComponent: root.weekHeaderDelegate
                    property var startDate: weekStartDate
                }
                Item {
                    id: dayDelegate
                    height: root.dayHeight
                    width: parent.width - weekHeader.width
                    property var startDate: weekStartDate
                    //Grid
                    Row {
                        height: parent.height
                        Repeater {
                            id: gridRepeater
                            model: root.daysPerRow
                            Item {
                                height: parent.height
                                width: root.dayWidth
                                property var date: DateUtils.addDaysToDate(dayDelegate.startDate, modelData)
                                property bool isInPast: DateUtils.roundToDay(date) < DateUtils.roundToDay(root.currentDate)
                                property bool isToday: DateUtils.sameDay(root.currentDate, date)

                                //Dimm days in the past
                                Rectangle {
                                    anchors.fill: parent
                                    color: Kube.Colors.buttonColor
                                    opacity: 0.2
                                    visible: isInPast
                                }

                                //Grid
                                Rectangle {
                                    anchors.fill: parent
                                    visible: root.paintGrid
                                    color: "transparent"
                                    border.width: 1
                                    border.color: Kube.Colors.lightgrey

                                }

                                //Day number
                                Label {
                                    visible: root.showDayIndicator
                                    anchors {
                                        top: parent.top
                                        left: parent.left
                                        topMargin: Kube.Units.smallSpacing
                                        leftMargin: Kube.Units.smallSpacing
                                    }
                                    text: date.getDate()
                                    font.bold: true
                                    color: isInPast ? Kube.Colors.disabledTextColor : Kube.Colors.textColor
                                    Rectangle {
                                        anchors {
                                            left: parent.left
                                            right: parent.right
                                            bottom: parent.bottom
                                        }
                                        width: Kube.Units.gridUnit
                                        height: 3
                                        color: Kube.Colors.plasmaBlue
                                        opacity: 0.6
                                        visible: isToday
                                    }
                                }
                            }
                        }
                    }

                    Column {
                        anchors {
                            fill: parent
                            //Offset for date
                            topMargin: root.showDayIndicator ? Kube.Units.gridUnit + Kube.Units.smallSpacing : 0
                        }
                        Repeater {
                            id: linesRepeater
                            model: events
                            onCountChanged: {
                                root.numberOfLinesShown = count
                            }
                            Item {
                                id: line
                                height: Kube.Units.gridUnit
                                width: parent.width

                                //Events
                                Repeater {
                                    id: eventsRepeater
                                    model: modelData
                                    Rectangle {
                                        x: root.dayWidth * modelData.starts
                                        y: 0
                                        width: root.dayWidth * modelData.duration
                                        height: parent.height

                                        color: modelData.color
                                        radius: 2
                                        border.width: 1
                                        border.color: Kube.Colors.viewBackgroundColor

                                        Kube.Label {
                                            anchors {
                                                fill: parent
                                                leftMargin: Kube.Units.smallSpacing
                                                rightMargin: Kube.Units.smallSpacing
                                            }
                                            color: Kube.Colors.highlightedTextColor
                                            text: modelData.text
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
