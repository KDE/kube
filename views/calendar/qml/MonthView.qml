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

    property int daysPerRow: 7
    property int daysToShow: daysPerRow * 5
    property var dayWidth: (root.width - Kube.Units.gridUnit  - Kube.Units.largeSpacing * 2) / root.daysPerRow
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
            fill: parent
            rightMargin: Kube.Units.largeSpacing
        }

        //FIXME weeknumber per row
        // Repeater {
        //     model: root.daysToShow / root.daysPerRow
        //     Item {
        //         id: weekNumber
        //         anchors {
        //             left: parent.left
        //         }
        //         y: index * root.dayHeight
        //         width: Kube.Units.gridUnit * 2
        //         height: Kube.Units.gridUnit * 2
        //         Label {
        //             anchors.centerIn: parent
        //             text: getWeek(startDate, 1)
        //             font.bold: true
        //         }
        //     }
        // }

        DayLabels {
            id: dayLabels
            anchors.top: parent.top
            anchors.right: parent.right
            startDate: root.startDate
            dayWidth: root.dayWidth
            daysToShow: root.daysPerRow
            showDate: false
        }

        MultiDayView {
            anchors {
                top: dayLabels.bottom
                right: parent.right
                bottom: parent.bottom
            }
            dayWidth: root.dayWidth
            daysToShow: root.daysToShow
            daysPerRow: root.daysPerRow
            currentDate: root.currentDate
            startDate: root.startDate
            calendarFilter: root.calendarFilter
            paintGrid: true
        }
    }
}
