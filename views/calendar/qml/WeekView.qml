/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
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

    property int daysToShow: 7
    property var dayWidth: (root.width - Kube.Units.gridUnit  - Kube.Units.largeSpacing * 2) / root.daysToShow
    property var hourHeight: Kube.Units.gridUnit * 2
    property date currentDate

    function getMonday(date) {
        var year = date.getFullYear()
        var month = date.getMonth()
        //Jup, getDate returns the day of the month
        var day = date.getDate()

        while (true) {
            if (date.getDay() === Locale.Monday) {
                return date
            }
            day = day - 1
            date = new Date(year, month, day)
        }
        return date
    }

    property date startDate: getMonday(currentDate)

    Item {
        anchors {
            top: parent.top
            right: parent.right
            rightMargin: Kube.Units.largeSpacing
        }

        width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2
        height: root.height

        //BEGIN day labels
        Row {
            id: dayLabels
            anchors.top: parent.top
            anchors.right: parent.right
            spacing: 0
            height: childrenRect.height
            width: root.dayWidth * root.daysToShow
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
        //END day labels

        //BEGIN daylong events
        Rectangle {
            id: daylong

            anchors {
                top: dayLabels.bottom
                right: parent.right
            }

            height: Kube.Units.gridUnit * 3
            width: root.dayWidth * root.daysToShow
            color: Kube.Colors.viewBackgroundColor
            border.width: 1
            border.color: Kube.Colors.buttonColor

            ListView {

                anchors {
                    fill: parent
                    margins: 1
                }

                clip: true

                model: Kube.DayLongEventModel {
                    start: root.startDate
                    length: root.daysToShow
                }

                delegate: Item {
                    height: Kube.Units.gridUnit + 2 // +2 to make good for the white border
                    width: daylong.width

                    Rectangle {
                        width: root.dayWidth * model.duration
                        height: parent.height
                        x: root.dayWidth * model.starts
                        color: model.color
                        border.width: 1
                        border.color: Kube.Colors.viewBackgroundColor

                        Kube.Label {
                            anchors {
                                left: parent.left
                                leftMargin: Kube.Units.smallSpacing
                            }
                            color: Kube.Colors.highlightedTextColor
                            text: model.summary
                        }
                    }
                }
            }

            //Dimm days in the past
            Rectangle {
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                width: (root.currentDate.getDate() - root.startDate.getDate()) * root.dayWidth
                color: Kube.Colors.buttonColor
                opacity: 0.2
            }
        }
        //END daylong events

        Flickable {
            id: mainWeekViewer

            anchors {
                top: daylong.bottom
            }

            Layout.fillWidth: true
            height: root.height - daylong.height - dayLabels.height - Kube.Units.largeSpacing
            width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2

            contentHeight: root.hourHeight * 24
            contentWidth: width

            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Kube.ScrollBar {}

            Row {
                height: root.hourHeight * 24
                width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2

                spacing: 0

                //BEGIN time labels
                Column {
                    anchors.bottom: parent.bottom
                    Repeater {
                        model: ["1:00","2:00","3:00","4:00","5:00","6:00","7:00","8:00","9:00","10:00","11:00","12:00",
                        "13:00","14:00","15:00","16:00","17:00","18:00","19:00","20:00","21:00","22:00","23:00","0:00"]
                        delegate: Item {
                            height: root.hourHeight
                            width: Kube.Units.gridUnit * 2

                            Kube.Label {
                                anchors {
                                    right: parent.right
                                    rightMargin: Kube.Units.smallSpacing
                                    bottom: parent.bottom
                                }
                                text: model.modelData
                            }
                        }
                    }
                }
                //END time labels

                Repeater {
                    model: Kube.PeriodDayEventModel {
                        start: root.startDate
                        length: root.daysToShow
                    }
                    delegate: Rectangle {
                        id: dayDelegate

                        property var events: model.events
                        property var date: model.date

                        width: root.dayWidth
                        height: root.hourHeight * 24

                        clip: true

                        color: Kube.Colors.viewBackgroundColor

                        //Grid
                        Column {
                            anchors.fill: parent
                            Repeater {
                                model: 12
                                delegate: Rectangle {
                                    height: root.hourHeight * 2
                                    width: parent.width
                                    color: "transparent"
                                    border.width: 1
                                    border.color: Kube.Colors.lightgrey
                                }
                            }
                        }

                        Repeater {
                            model: parent.events

                            delegate: Rectangle {
                                id: eventDelegate

                                states: [
                                State {
                                    name: "dnd"
                                    when: mouseArea.drag.active

                                    PropertyChanges {target: mouseArea; cursorShape: Qt.ClosedHandCursor}
                                    PropertyChanges {target: eventDelegate; x: x; y: y}
                                    PropertyChanges {target: eventDelegate; parent: root}
                                    PropertyChanges {target: eventDelegate; opacity: 0.7}
                                    PropertyChanges {target: eventDelegate; anchors.right: ""}
                                    PropertyChanges {target: eventDelegate; width: root.dayWidth - Kube.Units.smallSpacing * 2}
                                }
                                ]

                                anchors {
                                    right: parent.right
                                    rightMargin: Kube.Units.smallSpacing
                                }
                                radius: 2
                                width: root.dayWidth - Kube.Units.smallSpacing * 2 - Kube.Units.gridUnit * model.modelData.indentation
                                height: Math.max(root.hourHeight * 0.5, root.hourHeight * model.modelData.duration)
                                y: root.hourHeight * model.modelData.starts
                                x: Kube.Units.gridUnit * model.modelData.indentation

                                color: model.modelData.color
                                opacity: 0.8
                                border.width: 1
                                border.color: Kube.Colors.viewBackgroundColor

                                Kube.Label {
                                    anchors {
                                        left: parent.left
                                        leftMargin: Kube.Units.smallSpacing
                                    }
                                    text: model.modelData.text
                                    color: Kube.Colors.highlightedTextColor
                                }

                                Drag.active: mouseArea.drag.active
                                Drag.hotSpot.x: mouseArea.mouseX
                                Drag.hotSpot.y: mouseArea.mouseY
                                Drag.source: eventDelegate

                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent

                                    hoverEnabled: true
                                    drag.target: parent

                                    onEntered: {
                                        eventDelegate.z = eventDelegate.z + 100
                                    }
                                    onExited: {
                                        eventDelegate.z = eventDelegate.z - 100

                                    }
                                    onReleased: eventDelegate.Drag.drop()
                                }
                            }
                        }

                        Rectangle {
                            id: currentTimeLine
                            anchors {
                                right: parent.right
                                left: parent.left
                            }
                            y: root.hourHeight * root.currentDate.getHours() + root.hourHeight / 60 * root.currentDate.getMinutes()
                            height: 2
                            color: Kube.Colors.plasmaBlue
                            visible: root.currentDate.getDate() == dayDelegate.date.getDate()
                            opacity: 0.8
                        }

                        //Dimm days in the past
                        Rectangle {
                            anchors.fill: parent
                            color: Kube.Colors.buttonColor
                            opacity: 0.2
                            visible: root.currentDate.getDate() > dayDelegate.date.getDate()
                        }

                        DropArea {
                            anchors.fill: parent

                            onDropped: {
                                console.log("DROP")
                                drop.accept(Qt.MoveAction)
                                //drop.source.visible = false
                                console.log((drop.source.y - mainWeekViewer.y + mainWeekViewer.contentY) / hourHeight)
                            }
                        }
                    }
                }
            }
        }
    }
}
