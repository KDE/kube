/*
 *  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
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

Item {
    id: root

    height: button.height
    width: button.width

    Kube.Button {
        id: button

        text: selector.selectedDate.toLocaleDateString() //"Choose Date"

        onClicked: {
            popup.open()
        }

        Kube.Popup {
            id: popup

            property var year: 2018
            property var month: Calendar.April

            x: button.x
            y: button.y + button.height
            width: selector.implicitWidth + Kube.Units.largeSpacing * 2
            height: selector.implicitHeight + Kube.Units.largeSpacing * 2
            modal: true
            focus: true

            DateSelector {
                id: selector
                anchors.fill: parent
                selectedDate: new Date()
                backgroundColor: Kube.Colors.backgroundColor
                textColor: Kube.Colors.textColor
                invertIcons: false
            }
        }
    }
}
