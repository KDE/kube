/*
 *  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
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

import QtQuick 2.7
import QtQuick.Layouts 1.2
import org.kube.framework 1.0 as Kube
import Qt.labs.calendar 1.0

import "dateutils.js" as DateUtils

RowLayout {
    id: root
    property bool enableTime: true
    property var notBefore: new Date(0)
    property var initialValue: null

    property date dateTime: initialValue ? initialValue : new Date()

    spacing: Kube.Units.smallSpacing

    Component.onCompleted: {
        if (root.initialValue) {
            root.dateTime = root.initialValue
        }
    }


    Kube.Button {
        id: button

        Layout.preferredWidth: implicitWidth

        text: selector.selectedDate.toLocaleDateString()

        onClicked: {
            popup.open()
        }

        Kube.Popup {
            id: popup

            x: button.x
            y: button.y + button.height
            width: selector.implicitWidth + Kube.Units.largeSpacing * 2
            height: selector.implicitHeight + Kube.Units.largeSpacing * 2
            modal: true
            focus: true

            DateSelector {
                id: selector
                anchors.fill: parent
                notBefore: root.notBefore
                backgroundColor: Kube.Colors.backgroundColor
                textColor: Kube.Colors.textColor
                invertIcons: false
                selectedDate: root.dateTime
                onSelected: root.dateTime = date
                onNext: root.dateTime = DateUtils.nextMonth(selectedDate)
                onPrevious: root.dateTime = DateUtils.previousMonth(selectedDate)
            }
        }
    }

    TimeSelector {
        id: timeEdit
        Layout.preferredWidth: Kube.Units.gridUnit * 3
        notBefore: root.notBefore
        dateTime: root.dateTime
        onDateTimeChanged: {
           //Intermediate variable is necessary for binding to be updated
           var newDate = root.dateTime
           newDate.setHours(dateTime.getHours(), dateTime.getMinutes())
           root.dateTime = newDate
        }

        visible: root.enableTime
    }
}
