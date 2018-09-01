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

FocusScope {
    id: root

    property int daysToShow: 7
    property var dayWidth: (root.width - Kube.Units.gridUnit  - Kube.Units.largeSpacing * 2) / root.daysToShow
    property var hourHeight: Kube.Units.gridUnit * 2
    property date currentDate
    property date startDate: currentDate
    property var calendarFilter

    /**
    * Returns the week number for this date.  dowOffset is the day of week the week
    * "starts" on for your locale - it can be from 0 to 6. If dowOffset is 1 (Monday),
    * the week returned is the ISO 8601 week number.
    * @param int dowOffset
    * @return int
    */
    function getWeek(date, dowOffset) {
        var newYear = new Date(date.getFullYear(),0,1);
        var day = newYear.getDay() - dowOffset; //the day of week the year begins on
        day = (day >= 0 ? day : day + 7);
        var daynum = Math.floor((date.getTime() - newYear.getTime() - 
        (date.getTimezoneOffset()-newYear.getTimezoneOffset())*60000)/86400000) + 1;
        var weeknum;
        //if the year starts before the middle of a week
        if(day < 4) {
            weeknum = Math.floor((daynum+day-1)/7) + 1;
            if(weeknum > 52) {
                nYear = new Date(date.getFullYear() + 1,0,1);
                nday = nYear.getDay() - dowOffset;
                nday = nday >= 0 ? nday : nday + 7;
                /*if the next year starts before the middle of
                the week, it is week #1 of that year*/
                weeknum = nday < 4 ? 1 : 53;
            }
        }
        else {
            weeknum = Math.floor((daynum+day-1)/7);
        }
        return weeknum;
    }

    function roundToDay(date) {
        return new Date(date.getFullYear(), date.getMonth(), date.getDate())
    }

    Item {
        anchors {
            top: parent.top
            right: parent.right
            rightMargin: Kube.Units.largeSpacing
        }

        width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2
        height: root.height

        Item {
            id: weekNumber
            anchors {
                top: parent.top
                left: parent.left
            }
            width: Kube.Units.gridUnit * 2
            height: Kube.Units.gridUnit * 2
            Label {
                anchors.centerIn: parent
                text: getWeek(startDate, 1)
                font.bold: true
            }
        }

        DayLabels {
            id: dayLabels
            anchors.top: parent.top
            anchors.right: parent.right
            startDate: root.startDate
            dayWidth: root.dayWidth
            daysToShow: root.daysToShow
        }

        MultiDayView {
            id: daylong
            anchors {
                top: dayLabels.bottom
                right: parent.right
            }
            dayWidth: root.dayWidth
            daysToShow: root.daysToShow
            currentDate: root.currentDate
            startDate: root.startDate
            calendarFilter: root.calendarFilter
        }

        Flickable {
            id: mainWeekViewer

            anchors {
                top: daylong.bottom
            }

            Layout.fillWidth: true
            height: root.height - daylong.height - dayLabels.height - Kube.Units.largeSpacing
            width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2

            contentHeight: root.hourHeight * 24
            contentWidth: width

            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Kube.ScrollBar {}

            Kube.ScrollHelper {
                id: scrollHelper
                flickable: mainWeekViewer
                anchors.fill: parent
            }

            Row {
                height: root.hourHeight * 24
                width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2

                spacing: 0

                //BEGIN time labels
                Column {
                    anchors.bottom: parent.bottom
                    Repeater {
                        model: ["1:00","2:00","3:00","4:00","5:00","6:00","7:00","8:00","9:00","10:00","11:00","12:00",
                        "13:00","14:00","15:00","16:00","17:00","18:00","19:00","20:00","21:00","22:00","23:00","0:00"]
                        delegate: Item {
                            height: root.hourHeight
                            width: Kube.Units.gridUnit * 2

                            Kube.Label {
                                anchors {
                                    right: parent.right
                                    rightMargin: Kube.Units.smallSpacing
                                    bottom: parent.bottom
                                }
                                text: model.modelData
                            }
                        }
                    }
                }
                //END time labels

                Repeater {
                    model: Kube.PeriodDayEventModel {
                        model: Kube.EventModel {
                            start: root.startDate
                            length: root.daysToShow
                            calendarFilter: root.calendarFilter
                        }
                    }
                    delegate: Rectangle {
                        id: dayDelegate

                        width: root.dayWidth
                        height: root.hourHeight * 24

                        clip: true

                        color: Kube.Colors.viewBackgroundColor

                        property bool isInPast: roundToDay(root.currentDate) > roundToDay(date)
                        property bool isToday: roundToDay(root.currentDate).getTime() == roundToDay(date).getTime()

                        //Dimm days in the past
                        Rectangle {
                            anchors.fill: parent
                            color: Kube.Colors.buttonColor
                            opacity: 0.2
                            visible: isInPast
                        }

                        //Grid
                        Column {
                            anchors.fill: parent
                            Repeater {
                                model: 12
                                delegate: Rectangle {
                                    height: root.hourHeight * 2
                                    width: parent.width
                                    color: "transparent"
                                    border.width: 1
                                    border.color: Kube.Colors.lightgrey
                                }
                            }
                        }

                        Repeater {
                            model: events

                            delegate: Rectangle {
                                id: eventDelegate

                                states: [
                                State {
                                    name: "dnd"
                                    when: mouseArea.drag.active

                                    PropertyChanges {target: mouseArea; cursorShape: Qt.ClosedHandCursor}
                                    PropertyChanges {target: eventDelegate; x: x; y: y}
                                    PropertyChanges {target: eventDelegate; parent: root}
                                    PropertyChanges {target: eventDelegate; opacity: 0.7}
                                    PropertyChanges {target: eventDelegate; anchors.right: ""}
                                    PropertyChanges {target: eventDelegate; width: root.dayWidth - Kube.Units.smallSpacing * 2}
                                }
                                ]

                                anchors {
                                    right: parent.right
                                    rightMargin: Kube.Units.smallSpacing
                                }
                                radius: 2
                                width: root.dayWidth - Kube.Units.smallSpacing * 2 - Kube.Units.gridUnit * model.modelData.indentation
                                height: Math.max(root.hourHeight * 0.5, root.hourHeight * model.modelData.duration)
                                y: root.hourHeight * model.modelData.starts
                                x: Kube.Units.gridUnit * model.modelData.indentation

                                color: model.modelData.color
                                border.width: 1
                                border.color: Kube.Colors.viewBackgroundColor

                                Kube.Label {
                                    anchors {
                                        fill: parent
                                        leftMargin: Kube.Units.smallSpacing
                                        rightMargin: Kube.Units.smallSpacing
                                    }
                                    text: model.modelData.text
                                    color: Kube.Colors.highlightedTextColor
                                    wrapMode: Text.Wrap
                                    elide: Text.ElideRight
                                }

                                Drag.active: mouseArea.drag.active
                                Drag.hotSpot.x: mouseArea.mouseX
                                Drag.hotSpot.y: mouseArea.mouseY
                                Drag.source: eventDelegate

                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent

                                    hoverEnabled: true
                                    drag.target: parent

                                    onReleased: eventDelegate.Drag.drop()
                                }
                            }
                        }

                        Rectangle {
                            id: currentTimeLine
                            anchors {
                                right: parent.right
                                left: parent.left
                            }
                            y: root.hourHeight * root.currentDate.getHours() + root.hourHeight / 60 * root.currentDate.getMinutes()
                            height: 2
                            color: Kube.Colors.plasmaBlue
                            visible: isToday
                            opacity: 0.8
                        }


                        DropArea {
                            anchors.fill: parent

                            onDropped: {
                                console.log("DROP")
                                drop.accept(Qt.MoveAction)
                                //drop.source.visible = false
                                console.log((drop.source.y - mainWeekViewer.y + mainWeekViewer.contentY) / hourHeight)
                            }
                        }
                    }
                }
            }
        }
    }
}
