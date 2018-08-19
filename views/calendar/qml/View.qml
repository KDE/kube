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

import QtQuick 2.9
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.2

import org.kube.framework 1.0 as Kube


RowLayout {
    id: root

    property date currentDate: new Date()
    property date selectedDate: getFirstDayOfWeek(currentDate)
    property bool autoUpdateDate: true

    Timer {
        running: autoUpdateDate
        interval: 2000; repeat: true
        onTriggered: root.currentDate = new Date()
    }

    function getFirstDayOfWeek(date) {
        var firstDay = Qt.locale().firstDayOfWeek
        var year = date.getFullYear()
        var month = date.getMonth()
        //Jup, getDate returns the day of the month
        var day = date.getDate()

        while (true) {
            if (date.getDay() === firstDay) {
                return date
            }
            day = day - 1
            date = new Date(year, month, day)
        }
        return date
    }


    StackView.onActivated: {
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "event"})
    }

    Rectangle {
        anchors {
            top: parent.top
            bottom: parent.bottom
        }
        width: Kube.Units.gridUnit * 10
        color: Kube.Colors.darkBackgroundColor

        Column {
            id: topLayout
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: Kube.Units.largeSpacing
            }
            Kube.PositiveButton {
                id: newEventButton
                objectName: "newEventButton"

                anchors {
                    left: parent.left
                    right: parent.right
                }
                focus: true
                text: qsTr("New Event")
                onClicked: {}
            }
            DateView {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                date: root.currentDate
            }
        }

        ColumnLayout {
            //Grow from the button but don't go over topLayout
            anchors {
                bottom: parent.bottom
                left: topLayout.left
                right: parent.right
                bottomMargin: Kube.Units.largeSpacing
                rightMargin: Kube.Units.largeSpacing
            }
            height: Math.min(implicitHeight, parent.height - (topLayout.y + topLayout.height) - Kube.Units.largeSpacing)

            spacing: Kube.Units.largeSpacing

            DateSelector {
                id: dateSelector
                anchors {
                    left: parent.left
                    right: parent.right
                }
                selectedDate: root.selectedDate
                onSelectedDateChanged: {
                    root.selectedDate = getFirstDayOfWeek(dateSelector.selectedDate)
                    selectedDate = root.selectedDate
                }
            }

            Kube.ListView {
                id: listView
                Layout.fillHeight: true
                Layout.preferredHeight: contentHeight
                Layout.minimumHeight: Kube.Units.gridUnit
                anchors {
                    left: parent.left
                    right: parent.right
                }
                spacing: Kube.Units.smallSpacing
                model: Kube.CheckableEntityModel {
                    id: calendarModel
                    type: "calendar"
                    roles: ["name", "color"]
                }
                delegate: Item {
                    width: listView.availableWidth
                    height: Kube.Units.gridUnit
                    RowLayout {
                        anchors.fill: parent
                        spacing: Kube.Units.smallSpacing
                        Kube.CheckBox {
                            opacity: 0.9
                            checked: !model.checked
                            onToggled: model.checked = !checked
                        }
                        Kube.Label {
                            Layout.fillWidth: true
                            text: model.name
                            color: Kube.Colors.highlightedTextColor
                            elide: Text.ElideLeft
                            clip: true
                        }
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: Kube.Units.gridUnit
                            height: width
                            radius: width / 2
                            color: model.color
                            opacity: 0.9
                        }
                    }
                }
            }
        }
    }

    WeekView {
        Layout.fillHeight: true
        Layout.fillWidth: true
        currentDate: root.currentDate
        startDate: root.selectedDate
        calendarFilter: calendarModel.checkedEntities
    }
}
