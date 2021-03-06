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
    property double weekHeaderWidth: 0
    property double dayWidth: (width - weekHeaderWidth) / daysPerRow
    property date currentDate
    property date startDate
    property var calendarFilter
    property bool paintGrid: false
    property bool showDayIndicator: false
    property var filter
    property alias dayHeaderDelegate: dayLabels.delegate
    property Component weekHeaderDelegate
    property int month

    //Internal
    property int numberOfLinesShown: 0
    property int numberOfRows: (daysToShow / daysPerRow)
    property var dayHeight: (height - dayLabels.height) / numberOfRows

    implicitHeight: (numberOfRows > 1 ? Kube.Units.gridUnit * 10 * numberOfRows: numberOfLinesShown * Kube.Units.gridUnit) + dayLabels.height

    height: implicitHeight

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
                model: Kube.EventOccurrenceModel {
                    objectName: "eventOccurrenceModel"
                    start: root.startDate
                    length: root.daysToShow
                    calendarFilter: root.calendarFilter
                    filter: root.filter ? root.filter : {}
                }
                // daysPerRow: root.daysPerRow //Hardcoded to 7
            }
            //One row => one week
            Item {
                width: parent.width
                height: root.dayHeight
                clip: true
                Row {
                    width: parent.width
                    height: parent.height
                    Loader {
                        id: weekHeader
                        height: parent.height
                        sourceComponent: root.weekHeaderDelegate
                        property var startDate: weekStartDate
                        onStatusChanged: if (weekHeader.status == Loader.Ready) root.weekHeaderWidth = item.width
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
                                    property bool isCurrentMonth: date.getMonth() == root.month

                                    //Grid
                                    Rectangle {
                                        anchors.fill: parent
                                        visible: root.paintGrid
                                        color: Kube.Colors.viewBackgroundColor
                                        border.width: 1
                                        border.color: Kube.Colors.lightgrey

                                        //Dimm days in the past
                                        Rectangle {
                                            anchors.fill: parent
                                            anchors.margins: 1
                                            color: Kube.Colors.buttonColor
                                            opacity: 0.2
                                            visible: isInPast
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: Kube.Fabric.postMessage(Kube.Messages.eventEditor, {"start": date, "allDay": true})
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
                                        //We add the month abbrevation to the first of each month
                                        text: date.toLocaleDateString(Qt.locale(), date.getDate() == 1 ? "d MMM" : "d")
                                        font.bold: true
                                        color: isToday ? Kube.Colors.highlightColor : (!isCurrentMonth ? Kube.Colors.disabledTextColor : Kube.Colors.textColor)
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

                                            radius: 2

                                            Rectangle {
                                                anchors.fill: parent
                                                color: modelData.color
                                                radius: 2
                                                border.width: 1
                                                border.color: Kube.Colors.viewBackgroundColor
                                                opacity: 0.6
                                            }

                                            Kube.Label {
                                                anchors {
                                                    fill: parent
                                                    leftMargin: Kube.Units.smallSpacing
                                                    rightMargin: Kube.Units.smallSpacing
                                                }
                                                color: Kube.Colors.textColor
                                                text: modelData.text
                                                elide: Text.ElideRight
                                            }

                                            MouseArea {
                                                id: mouseArea
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                onClicked: eventDetails.createObject(root, {}).open()
                                                Component {
                                                    id: eventDetails
                                                    Kube.Popup {
                                                        id: popup
                                                        parent: ApplicationWindow.overlay
                                                        x: Math.round((parent.width - width) / 2)
                                                        y: Math.round((parent.height - height) / 2)
                                                        width: eventView.width
                                                        height: eventView.height
                                                        padding: 0
                                                        EventView {
                                                            id: eventView
                                                            controller: Kube.EventController {
                                                                eventOccurrence: model.modelData.eventOccurrence
                                                            }
                                                            onDone: popup.close()
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
            }
        }
    }
}
