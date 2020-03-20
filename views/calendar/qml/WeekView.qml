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

    property int daysToShow: 7
    property var dayWidth: (root.width - Kube.Units.gridUnit - Kube.Units.largeSpacing) / root.daysToShow
    property var hourHeight: Kube.Units.gridUnit * 2
    property date currentDate
    property date startDate: currentDate
    property var calendarFilter

    Item {
        anchors {
            top: parent.top
            right: parent.right
        }

        width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2
        height: root.height

        Item {
            id: weekNumber
            anchors {
                top: parent.top
                left: parent.left
            }
            width: Kube.Units.gridUnit * 2
            height: Kube.Units.gridUnit * 2
            Label {
                anchors.centerIn: parent
                text: DateUtils.getWeek(startDate, Qt.locale().firstDayOfWeek)
                font.bold: true
            }
        }

        MultiDayView {
            id: daylong
            objectName: "weekView"
            anchors {
                top: parent.top
                right: parent.right
                left: parent.left
                leftMargin: Kube.Units.gridUnit * 2
            }

            dayWidth: root.dayWidth
            daysToShow: root.daysToShow
            currentDate: root.currentDate
            startDate: root.startDate
            calendarFilter: root.calendarFilter
            filter: {"allDay": true}
            paintGrid: true
            showDayIndicator: false
            dayHeaderDelegate: Item {
                height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 3
                Column {
                    anchors.centerIn: parent
                    Kube.Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.bold: true
                        text: day.toLocaleString(Qt.locale(), "dddd")
                    }
                    Kube.Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: day.toLocaleString(Qt.locale(), "d")
                        color: Kube.Colors.disabledTextColor
                        font.pointSize: Kube.Units.tinyFontSize
                    }
                }
            }
        }

        Flickable {
            id: mainWeekViewer

            anchors {
                top: daylong.bottom
            }

            Layout.fillWidth: true
            height: root.height - daylong.height - Kube.Units.largeSpacing
            width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2

            contentHeight: root.hourHeight * 24
            contentWidth: width

            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Kube.ScrollBar {}

            Kube.ScrollHelper {
                id: scrollHelper
                flickable: mainWeekViewer
                anchors.fill: parent
            }

            Row {
                height: root.hourHeight * 24
                width: root.dayWidth * root.daysToShow + Kube.Units.gridUnit * 2

                spacing: 0

                //BEGIN time labels
                Column {
                    anchors {
                        bottom: parent.bottom
                        //offset so the label is center aligned to the line
                        bottomMargin: root.hourHeight - Kube.Units.gridUnit / 2
                    }
                    Repeater {
                        model: ["1:00","2:00","3:00","4:00","5:00","6:00","7:00","8:00","9:00","10:00","11:00","12:00",
                        "13:00","14:00","15:00","16:00","17:00","18:00","19:00","20:00","21:00","22:00","23:00"]
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
                        model: Kube.EventOccurrenceModel {
                            start: root.startDate
                            length: root.daysToShow
                            calendarFilter: root.calendarFilter
                        }
                    }
                    delegate: Rectangle {
                        id: dayDelegate

                        width: root.dayWidth
                        height: root.hourHeight * 24


                        color: Kube.Colors.viewBackgroundColor

                        property bool isInPast: DateUtils.roundToDay(root.currentDate) > DateUtils.roundToDay(date)
                        property bool isToday: DateUtils.sameDay(root.currentDate, date)
                        property var todaysDate: date

                        //Dimm days in the past
                        Rectangle {
                            anchors.fill: parent
                            color: Kube.Colors.buttonColor
                            opacity: 0.2
                            visible: isInPast
                        }

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
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var d = dayDelegate.todaysDate
                                            var hours = index * 2
                                            var minuteOffset = 120 / parent.height * mouse.y
                                            var minutes = minuteOffset % 60
                                            hours += (minuteOffset - minutes) / 60
                                            d.setHours(hours)
                                            d.setMinutes(minutes)
                                            Kube.Fabric.postMessage(Kube.Messages.eventEditor, {"start": d, "allDay": false})
                                        }
                                    }
                                }
                            }
                        }

                        Repeater {
                            model: events

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
                                z: model.modelData.indentation

                                Rectangle {
                                    anchors.fill: parent
                                    color: model.modelData.color
                                    opacity: 0.6
                                    border.width: 1
                                    border.color: Kube.Colors.viewBackgroundColor
                                    radius: 2
                                }

                                Column {
                                    anchors {
                                        top: parent.top
                                        left: parent.left
                                        right: parent.right
                                        bottom: parent.bottom
                                        leftMargin: Kube.Units.smallSpacing
                                        rightMargin: Kube.Units.smallSpacing
                                    }
                                    Kube.Label {
                                        anchors {
                                            left: parent.left
                                            right: parent.right
                                        }
                                        text: model.modelData.text
                                        color: Kube.Colors.textColor
                                        wrapMode: Text.Wrap
                                        elide: Text.ElideRight
                                        //Only show two lines if we have either space for three or there is no dateLabel
                                        maximumLineCount: model.modelData.duration >= (dateLabel.visible ? 2.0 : 1.0) ? 2 : 1
                                    }
                                    Kube.Label {
                                        id: dateLabel
                                        anchors {
                                            left: parent.left
                                            right: parent.right
                                        }
                                        visible: model.modelData.duration >= 1.0 && model.modelData.startDate
                                        text: model.modelData.startDate.toLocaleString(Qt.locale(), "hh:mm")
                                        font.pointSize: Kube.Units.smallFontSize
                                        color: Kube.Colors.textColor
                                        elide: Text.ElideRight
                                    }
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

                                    onReleased: eventDelegate.Drag.drop()
                                    onClicked: eventDetails.createObject(root, {}).open()

                                    Component {
                                        id: eventDetails
                                        Kube.Popup {
                                            id: popup
                                            parent: ApplicationWindow.overlay
                                            x: Math.round((parent.width - width) / 2)
                                            y: Math.round((parent.height - height) / 2)
                                            width: Math.min(eventView.implicitWidth, parent.width - 2 * Kube.Units.gridUnit)
                                            height: Math.min(eventView.implicitHeight, parent.height - 2 * Kube.Units.gridUnit)
                                            padding: 0
                                            EventView {
                                                id: eventView
                                                anchors.fill: parent
                                                controller: Kube.EventController {
                                                    eventOccurrence: model.modelData.eventOccurrence
                                                }
                                                onDone: popup.close()
                                            }
                                        }
                                    }
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
                            visible: isToday
                            opacity: 0.8
                            Rectangle {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    left: parent.left
                                    leftMargin: -(width / 2)
                                }
                                width: Kube.Units.gridUnit / 2
                                height: width
                                radius: width / 2
                                color: Kube.Colors.plasmaBlue
                                opacity: 1
                            }
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
