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

FocusScope {
    id: root

    property int daysPerRow: 7
    property int daysToShow: daysPerRow * 6
    property var dayWidth: (root.width - Kube.Units.gridUnit  - Kube.Units.largeSpacing * 2) / root.daysPerRow
    property var hourHeight: Kube.Units.gridUnit * 2
    property date currentDate
    property date startDate: currentDate
    property var calendarFilter



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
        //             text: DateUtils.getWeek(startDate, Qt.locale().firstDayOfWeek)
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
