/*
 *  Copyright (C) 2019 Christian Mollekopf, <mollekopf@kolabsys.com>
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

import QtQuick 2
import QtQuick.Layouts 1
import QtQuick.Controls 2
import org.kube.framework 1.0 as Kube

ColumnLayout {
    id: root

    property alias initialStart: startDate.initialValue
    property alias initialEnd: endDate.initialValue
    property alias allDay: checkBox.checked
    property date start
    property date end

    spacing: Kube.Units.smallSpacing


    RowLayout {
        spacing: Kube.Units.smallSpacing
        Kube.Label {
            text: qsTr("from")
        }
        DateTimeChooser {
            id: startDate
            objectName: "startDate"
            enableTime: !root.allDay
            onDateTimeChanged: root.start = dateTime
        }
        RowLayout {
            spacing: Kube.Units.smallSpacing
            Kube.CheckBox {
                id: checkBox
            }
            Kube.Label {
                text: qsTr("All day")
            }
        }
    }
    RowLayout {
        spacing: Kube.Units.smallSpacing
        Kube.Label {
            text: qsTr("until")
        }
        DateTimeChooser {
            id: endDate
            objectName: "endDate"
            enableTime: !root.allDay
            notBefore: startDate.dateTime
            onDateTimeChanged: root.end = dateTime
        }
    }
}
