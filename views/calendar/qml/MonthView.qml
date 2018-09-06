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

    Kube.PeriodDayEventModel {
        id: eventModel
        start: root.startDate
        length: root.daysToShow
        calendarFilter: root.calendarFilter
    }

    Item {
        anchors {
            top: parent.top
            right: parent.right
            rightMargin: Kube.Units.largeSpacing
        }

        width: root.width
        height: root.height

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

            GridLayout {

                columns: 2

                DayOfWeekRow {
                    locale: grid.locale

                    Layout.column: 1
                    Layout.fillWidth: true
                }

                WeekNumberColumn {
                    month: grid.month
                    year: grid.year
                    locale: grid.locale

                    Layout.fillHeight: true
                }

                MonthGrid {
                    id: grid
                    month: popup.month
                    year: popup.year
                    locale: Qt.locale("en_GB") //FIXME

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    delegate: Kube.AbstractButton {
                        text: model.day

                        width: Kube.Units.gridUnit * 3
                    }
                }
            }
        }
    }
}
