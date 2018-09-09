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

    property alias startDate: dayView.startDate
    property alias currentDate: dayView.currentDate
    property alias calendarFilter: dayView.calendarFilter

    MultiDayView {
        id: dayView
        anchors {
            fill: parent
            rightMargin: Kube.Units.gridUnit
        }
        daysToShow: daysPerRow * 6
        daysPerRow: 7
        currentDate: root.currentDate
        startDate: root.startDate
        calendarFilter: root.calendarFilter
        paintGrid: true
        showDayIndicator: true
        dayHeaderDelegate: Item {
            height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 3
            Column {
                anchors.centerIn: parent
                Kube.Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                    text: day.toLocaleString(Qt.locale(), "dddd")
                }
            }
        }
        weekHeaderDelegate: Item {
            width: Kube.Units.gridUnit
            Kube.Label {
                anchors.centerIn: parent
                font.bold: true
                text: DateUtils.getWeek(startDate, Qt.locale().firstDayOfWeek)
                color: Kube.Colors.disabledTextColor
            }
        }
    }
}
