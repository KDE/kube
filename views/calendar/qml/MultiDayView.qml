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

Rectangle {
    id: root
    property int daysToShow
    property var dayWidth
    property date currentDate
    property date startDate
    property var calendarFilter

    //Internal
    property int numberOfLinesShown: 0

    width: root.dayWidth * root.daysToShow
    color: Kube.Colors.viewBackgroundColor
    border.width: 1
    border.color: Kube.Colors.buttonColor

    //+2 to compensate for borders
    height: numberOfLinesShown * Kube.Units.gridUnit + 2
    visible: numberOfLinesShown

    //Dimm days in the past
    Rectangle {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        //if more than 7 days in past, set to 7, otherwise actual number of days in the past
        width: (new Date(root.startDate.getFullYear(), root.startDate.getMonth(), root.startDate.getDate() + 7) < roundToDay(root.currentDate) ? 7 : root.currentDate.getDate() - root.startDate.getDate()) * root.dayWidth
        color: Kube.Colors.buttonColor
        opacity: 0.2
        //Avoid showing at all in the future (the width calculation will not work either)
        visible: roundToDay(root.currentDate) >= roundToDay(root.startDate)
    }

    Column {
        id: fullDayListView
        anchors {
            fill: parent
            margins: 1
        }
        //Lines
        Repeater {
            id: daysRepeater
            model: Kube.MultiDayEventModel {
                model: Kube.EventModel {
                    start: root.startDate
                    length: root.daysToShow
                    calendarFilter: root.calendarFilter
                }
            }
            Repeater {
                id: linesRepeater
                model: events
                onCountChanged: {
                    console.warn("eventcount ", count)
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
