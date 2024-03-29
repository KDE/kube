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
import QtQuick.Layouts 1.3

import "dateutils.js" as DateUtils

import org.kube.framework 1.0 as Kube

Item {
    id: root
    property date selectedDate
    property var selectedEnd: null
    property date notBefore: new Date(0) //Earliest possible by epoch
    property color backgroundColor: Kube.Colors.darkBackgroundColor
    property color textColor: Kube.Colors.highlightedTextColor
    property bool invertIcons: true
    property bool rangeSelection: false


    // Private
    property date currentDate: selectedDate

    function next() {
        root.currentDate = DateUtils.nextMonth(root.currentDate)
    }
    function previous() {
        root.currentDate = DateUtils.previousMonth(root.currentDate)
    }
    signal selected(date date)
    signal endSelected(date date)

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
                text: root.currentDate.toLocaleString(Qt.locale(), "MMMM yyyy")
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

            month: root.currentDate.getMonth()
            year: root.currentDate.getFullYear()
            locale: Qt.locale()

            contentItem: GridLayout {
                rows: 6
                columns: 7
                rowSpacing: grid.spacing
                columnSpacing: grid.spacing

                Repeater {
                    model: grid.source
                    delegate: grid.delegate
                }
            }

            delegate: Text {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                opacity: (model.month === grid.month && model.date.getTime() >= root.notBefore.getTime()) ? 1 : 0.5
                text: model.day
                font: grid.font
                color: root.textColor
                Layout.preferredHeight: Kube.Units.gridUnit / 4 * 3
                Layout.fillWidth: true

                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    function dateIsInRange(day, month, selectedDate, selectedEnd) {
                        const startMonth = selectedDate.getMonth();
                        const startDay = selectedDate.getDate();
                        if (day === startDay && month === startMonth) {
                            return true;
                        }
                        if (selectedEnd) {
                            const endMonth = selectedEnd.getMonth();
                            const endDay = selectedEnd.getDate();
                            if (
                                (month < endMonth && month === startMonth && day >= startDay) ||
                                (month > startMonth && month < endMonth) ||
                                (month > startMonth && month === endMonth && day <= endDay) ||
                                (month === startMonth && day >= startDay && day <= endDay)
                            ) {
                                return true;
                            }
                        }
                        return false;
                    }
                    height: 2
                    color: Kube.Colors.plasmaBlue
                    opacity: 0.6
                    visible: dateIsInRange(model.day, model.month, root.selectedDate, root.selectedEnd)
                }
            }

            onClicked: {
                if (
                    root.rangeSelection && root.selectedDate && !root.selectedEnd &&
                    date.getTime() >= root.selectedDate.getTime()
                ) {
                    //Select the end of the range
                    root.selectedEnd = date
                    root.endSelected(date)
                } else {
                    if (date.getTime() >= root.notBefore.getTime()) {
                        //Set the start
                        root.selectedEnd = null
                        root.selected(date)
                        root.endSelected(date)
                    }
                }
            }
        }
    }
}
