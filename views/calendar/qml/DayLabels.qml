/*
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

Row {
    id: root
    property date startDate
    property int dayWidth
    property int daysToShow

    height: childrenRect.height
    width: dayWidth * daysToShow

    spacing: 0
    Repeater {
        model: root.daysToShow
        delegate: Item {
            width: root.dayWidth
            height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 3
            Kube.Label {
                function addDaysToDate(date, days) {
                    var date = new Date(date);
                    date.setDate(date.getDate() + days);
                    return date;
                }
                font.bold: true

                anchors.centerIn: parent
                text: addDaysToDate(root.startDate, modelData).toLocaleString(Qt.locale(), "dddd")
            }
        }
    }
}
