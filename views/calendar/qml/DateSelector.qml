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
import QtQuick.Controls 2.2
import Qt.labs.calendar 1.0

import org.kube.framework 1.0 as Kube

Item {
    id: root
    property date selectedDate
    property date notBefore: new Date(0) //Earliest possible by epoch
    property color backgroundColor: Kube.Colors.darkBackgroundColor
    property color textColor: Kube.Colors.highlightedTextColor
    property bool invertIcons: true

    signal next()
    signal previous()
    signal selected(date date)

    implicitWidth: Math.max(grid.implicitWidth, dateLabel.implicitWidth + 2 * Kube.Units.gridUnit)
    implicitHeight: column.implicitHeight

    onNotBeforeChanged: {
        if (notBefore.getTime() > selectedDate.getTime()) {
            root.selected(notBefore)
        }
    }

    Column {
        id: column
        anchors.fill: parent
        spacing: Kube.Units.smallSpacing
        Item {
            anchors {
                left: parent.left
                right: parent.right
            }
            height: Kube.Units.gridUnit
            Kube.IconButton {
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                }
                height: parent.height
                width: parent.height
                color: root.backgroundColor
                iconName: Kube.Icons.iconName(Kube.Icons.goBack, root.invertIcons)
                visible: root.notBefore.getTime() < (new Date(root.selectedDate.getFullYear(), root.selectedDate.getMonth(), root.selectedDate.getDate())).getTime()
                onClicked: {
                    root.previous()
                }
            }
            Kube.Label {
                id: dateLabel
                anchors {
                    verticalCenter: parent.verticalCenter
                    horizontalCenter: parent.horizontalCenter
                }
                color: root.textColor
                font.bold: true
                text: root.selectedDate.toLocaleString(Qt.locale(), "MMMM yyyy")
            }
            Kube.IconButton {
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }
                height: parent.height
                width: parent.height
                color: root.backgroundColor
                iconName: Kube.Icons.iconName(Kube.Icons.goNext, root.invertIcons)
                onClicked: {
                    root.next()
                }
            }
        }

        MonthGrid {
            id: grid
            anchors {
                left: parent.left
                right: parent.right
            }

            month: root.selectedDate.getMonth()
            year: root.selectedDate.getFullYear()
            locale: Qt.locale()

            delegate: Text {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                opacity: (model.month === grid.month && model.date.getTime() >= root.notBefore.getTime()) ? 1 : 0.5
                text: model.day
                font: grid.font
                color: root.textColor
                height: implicitHeight

                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    width: Kube.Units.gridUnit
                    height: 3
                    color: Kube.Colors.plasmaBlue
                    opacity: 0.6
                    visible: model.day === root.selectedDate.getDate() && model.month === root.selectedDate.getMonth()
                }
            }

            onClicked: {
                if (date.getTime() >= root.notBefore.getTime()) {
                    root.selected(date)
                }
            }
        }
    }
}
