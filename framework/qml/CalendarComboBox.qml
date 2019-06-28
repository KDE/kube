/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
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

import QtQuick 2.4

import org.kube.framework 1.0 as Kube

Kube.ComboBox {
    id: root

    property alias accountId: calendarModel.accountId
    property string contentType: "event"

    signal selected(var calendar)

    model: Kube.EntityModel {
        id: calendarModel
        type: "calendar"
        roles: ["name", "color"]
        sortRole: "name"
        filter: {"contentTypes": contentType, "enabled": true}
        //Set initial selection.
        //onCurrentIndexChanged will not work because the as more items are added the currentIndex changes,
        //but depending on the sorting it will point to a different item (Which is really a bug of the model or ComboBox).
        onInitialItemsLoaded: {
            if (currentIndex >= 0) {
                root.selected(calendarModel.data(currentIndex).object)
            }
        }
    }

    textRole: "name"

    onCurrentIndexChanged: {
        if (currentIndex >= 0) {
            root.selected(calendarModel.data(currentIndex).object)
        }
    }

    delegate: Kube.ListDelegate {
        width: root.popup.width
        height: Kube.Units.gridUnit * 1.5

        contentItem: Row {
            Item {
                width: Kube.Units.smallSpacing
                height: parent.height
            }
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: Kube.Units.gridUnit
                height: Kube.Units.gridUnit
                radius: Kube.Units.gridUnit / 2
                color: model.color
            }
            Kube.Label {
                padding: Kube.Units.smallSpacing
                text: model[root.textRole]
                color:  root.highlightedIndex === index ? Kube.Colors.highlightedTextColor : Kube.Colors.textColor
            }
        }

        MouseArea {
            anchors.fill: parent

            onClicked: {
                root.currentIndex = calendarSelector.highlightedIndex
                root.popup.close()
            }
        }
    }
}
