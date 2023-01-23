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

import QtQuick 2
import QtQuick.Layouts 1
import QtQuick.Controls 2

import org.kube.framework 1.0 as Kube
import "dateutils.js" as DateUtils

Item {
    id: root
    property int daysToShow
    property int daysPerRow: daysToShow
    property double weekHeaderWidth: 0
    property double dayWidth: (width - weekHeaderWidth) / daysPerRow
    property date currentDate
    property date startDate
    property var calendarFilter
    property bool paintGrid: false
    property bool showDayIndicator: false
    property var filter
    property alias dayHeaderDelegate: dayLabels.delegate
    property Component weekHeaderDelegate
    property int month

    //Internal
    readonly property int verticalSpacing: 2
    property int horizontalSpacing: Kube.Units.smallSpacing
    readonly property int lineHeight: Kube.Units.gridUnit
    property int numberOfLinesShown: 0
    readonly property int numberOfRows: (daysToShow / daysPerRow)
    property var dayHeight: (height - dayLabels.height) / numberOfRows

    implicitHeight: (numberOfRows > 1 ? Kube.Units.gridUnit * 10 * numberOfRows: numberOfLinesShown * (Kube.Units.gridUnit + verticalSpacing)) + dayLabels.height + root.verticalSpacing

    height: implicitHeight

    ButtonGroup {
        id: expandButtonGroup
        exclusive: false
        //Like exclusive, but a checked button can be unchecked by clicking again.
        onClicked: {
            for (var i = 0; i < buttons.length; i++){
                var btn = buttons[i];
                if (button != btn) {
                    btn.checked = false;
                }
            }
        }
    }

    ColumnLayout {
        anchors {
            fill: parent
        }
        spacing: 0

        DayLabels {
            id: dayLabels
            startDate: root.startDate
            dayWidth: root.dayWidth
            daysToShow: root.daysPerRow
        }

        //Weeks
        Repeater {
            model: Kube.MultiDayEventModel {
                model: Kube.EventOccurrenceModel {
                    objectName: "eventOccurrenceModel"
                    start: root.startDate
                    length: root.daysToShow
                    calendarFilter: root.calendarFilter
                    filter: root.filter ? root.filter : ({})
                }
                // daysPerRow: root.daysPerRow //Hardcoded to 7
            }
            //One row => one week
            Item {
                id: weekRow
                Layout.fillHeight: true
                Layout.minimumHeight: expanded ? calculatedHeight : Kube.Units.gridUnit + Kube.Units.smallSpacing
                Layout.preferredHeight: root.dayHeight
                implicitWidth: parent.width
                property bool expanded: expandButton.checked
                readonly property int calculatedHeight: Math.max(
                    root.dayHeight,
                    events.length * (root.lineHeight + root.verticalSpacing) + (root.showDayIndicator ? Kube.Units.gridUnit + root.verticalSpacing : root.verticalSpacing)
                )
                property bool overfilled: calculatedHeight > root.dayHeight

                clip: true
                Row {
                    width: parent.width
                    height: parent.height
                    Loader {
                        id: weekHeader
                        height: parent.height
                        sourceComponent: root.weekHeaderDelegate
                        property var startDate: weekStartDate
                        onStatusChanged: if (weekHeader.status == Loader.Ready) root.weekHeaderWidth = item.width
                    }
                    Item {
                        id: dayDelegate
                        height: parent.height
                        width: parent.width - weekHeader.width
                        property var startDate: weekStartDate
                        //Grid
                        Row {
                            height: parent.height
                            Repeater {
                                id: gridRepeater
                                model: root.daysPerRow
                                Item {
                                    height: parent.height
                                    width: root.dayWidth
                                    property var date: DateUtils.addDaysToDate(dayDelegate.startDate, modelData)
                                    property bool isInPast: DateUtils.roundToDay(date) < DateUtils.roundToDay(root.currentDate)
                                    property bool isToday: DateUtils.sameDay(root.currentDate, date)
                                    property bool isCurrentMonth: date.getMonth() == root.month

                                    //Grid
                                    Rectangle {
                                        anchors.fill: parent
                                        visible: root.paintGrid
                                        color: Kube.Colors.viewBackgroundColor
                                        border.width: 1
                                        border.color: Kube.Colors.lightgrey

                                        //Dimm days in the past
                                        Rectangle {
                                            anchors.fill: parent
                                            anchors.margins: 1
                                            color: isToday && root.showDayIndicator ? Kube.Colors.activeBackgroundColor : Kube.Colors.buttonColor
                                            opacity: isToday && root.showDayIndicator ? 0.2 : 0.4
                                            visible: isInPast || (isToday && root.showDayIndicator)
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: Kube.Fabric.postMessage(Kube.Messages.eventEditor, {"start": date, "allDay": true})
                                    }

                                    //Day number
                                    Label {
                                        visible: root.showDayIndicator
                                        anchors {
                                            top: parent.top
                                            left: parent.left
                                            topMargin: Kube.Units.smallSpacing
                                            leftMargin: Kube.Units.smallSpacing
                                        }
                                        //We add the month abbrevation to the first of each month
                                        text: date.toLocaleDateString(Qt.locale(), date.getDate() == 1 ? "d MMM" : "d")
                                        font.bold: true
                                        color: isToday ? Kube.Colors.highlightColor : (!isCurrentMonth ? Kube.Colors.disabledTextColor : Kube.Colors.textColor)
                                    }
                                }
                            }
                        }

                        Column {
                            anchors {
                                fill: parent
                                //Offset for date
                                topMargin: root.showDayIndicator ? Kube.Units.gridUnit + root.verticalSpacing : root.verticalSpacing
                            }

                            spacing: root.verticalSpacing

                            Repeater {
                                id: linesRepeater
                                model: events
                                onCountChanged: {
                                    root.numberOfLinesShown = count
                                }
                                Item {
                                    id: line
                                    height: root.lineHeight
                                    width: parent.width

                                    //Events
                                    Repeater {
                                        id: eventsRepeater
                                        model: modelData
                                        Rectangle {
                                            x: root.dayWidth * modelData.starts + root.horizontalSpacing
                                            y: 0
                                            width: root.dayWidth * modelData.duration - (2 * root.horizontalSpacing)
                                            height: parent.height

                                            radius: 2

                                            Rectangle {
                                                anchors.fill: parent
                                                color: modelData.color
                                                radius: parent.radius
                                                border.width: 1
                                                border.color: Kube.Colors.viewBackgroundColor
                                                opacity: 0.6
                                            }

                                            Kube.Label {
                                                anchors {
                                                    fill: parent
                                                    leftMargin: Kube.Units.smallSpacing
                                                    rightMargin: Kube.Units.smallSpacing
                                                }
                                                color: Kube.Colors.textColor
                                                text: modelData.text
                                                elide: Text.ElideRight
                                                font.weight: Font.Medium
                                            }

                                            MouseArea {
                                                id: mouseArea
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                onClicked: eventDetails.createObject(root, {}).open()
                                                Component {
                                                    id: eventDetails
                                                    Kube.Popup {
                                                        id: popup
                                                        parent: ApplicationWindow.overlay
                                                        x: Math.round((parent.width - width) / 2)
                                                        y: Math.round((parent.height - height) / 2)
                                                        width: eventView.implicitWidth
                                                        height: eventView.implicitWidth
                                                        padding: 0
                                                        Rectangle {
                                                            anchors.fill: parent
                                                            color: Kube.Colors.paperWhite
                                                            EventEditor {
                                                                id: eventView
                                                                anchors.fill: parent
                                                                editMode: true
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
                                    }
                                }
                            }
                        }
                    }
                }
                Kube.IconButton {
                    id: expandButton
                    visible: weekRow.overfilled
                    anchors {
                        bottom: parent.bottom
                        horizontalCenter: parent.horizontalCenter
                    }
                    checkable: true
                    iconName: checked ? Kube.Icons.goUp : Kube.Icons.goDown
                    ButtonGroup.group: expandButtonGroup
                }
            }
        }
    }
}
