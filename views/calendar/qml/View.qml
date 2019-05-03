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

import QtQuick 2.9
import QtQuick.Controls 2
import QtQuick.Layouts 1.2

import org.kube.framework 1.0 as Kube
import "dateutils.js" as DateUtils

Kube.View {
    id: root

    property date currentDate: new Date()
    property date selectedDate: currentDate
    property bool autoUpdateDate: true

    onSelectedDateChanged: {
        console.log("Selected date changed", selectedDate)
    }

    onRefresh: {
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "calendar"})
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "event"})
    }

    RowLayout {

        Timer {
            running: autoUpdateDate
            interval: 2000; repeat: true
            onTriggered: root.currentDate = new Date()
        }

        Rectangle {
            Layout.fillHeight: true
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
                spacing: Kube.Units.largeSpacing
                Kube.PositiveButton {
                    id: newEventButton
                    objectName: "newEventButton"

                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    focus: true
                    text: qsTr("New Event")
                    onClicked: eventPopup.createObject(root, {start: DateUtils.sameDay(root.currentDate, root.selectedDate) ? root.currentDate : root.selectedDate}).open()
                }
                RowLayout {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    spacing: Kube.Units.smallSpacing
                    ButtonGroup {
                        id: viewButtonGroup
                    }
                    Kube.TextButton {
                        id: weekViewButton
                        Layout.fillWidth: true
                        text: qsTr("Week")
                        textColor: Kube.Colors.highlightedTextColor
                        checkable: true
                        checked: true
                        ButtonGroup.group: viewButtonGroup
                    }
                    Kube.TextButton {
                        id: monthViewButton
                        Layout.fillWidth: true
                        text: qsTr("Month")
                        textColor: Kube.Colors.highlightedTextColor
                        checkable: true
                        ButtonGroup.group: viewButtonGroup
                    }
                }
                DateView {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    date: root.currentDate
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.selectedDate = root.currentDate
                        }
                    }
                }

                DateSelector {
                    id: dateSelector
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    selectedDate: root.selectedDate
                    onSelected: {
                        root.selectedDate = date
                    }
                    onNext: {
                        if (weekViewButton.checked) {
                            root.selectedDate = DateUtils.nextWeek(root.selectedDate)
                        } else {
                            root.selectedDate = DateUtils.nextMonth(root.selectedDate)
                        }
                    }
                    onPrevious: {
                        if (weekViewButton.checked) {
                            root.selectedDate = DateUtils.previousWeek(root.selectedDate)
                        } else {
                            root.selectedDate = DateUtils.previousMonth(root.selectedDate)
                        }
                    }
                }
            }

            Kube.CalendarSelector {
                id: accountSwitcher

                //Grow from the button but don't go over topLayout
                anchors {
                    bottom: statusBarContainer.top
                    left: topLayout.left
                    right: parent.right
                    bottomMargin: Kube.Units.largeSpacing
                    rightMargin: Kube.Units.largeSpacing
                }

                height: parent.height - (topLayout.y + topLayout.height) - Kube.Units.largeSpacing - anchors.bottomMargin - statusBarContainer.height
            }

            Item {
                id: statusBarContainer
                anchors {
                    topMargin: Kube.Units.smallSpacing
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                height: childrenRect.height

                Rectangle {
                    id: border
                    visible: statusBar.visible
                    anchors {
                        right: parent.right
                        left: parent.left
                        margins: Kube.Units.smallSpacing
                    }
                    height: 1
                    color: Kube.Colors.viewBackgroundColor
                    opacity: 0.3
                }
                Kube.StatusBar {
                    id: statusBar
                    accountId: accountSwitcher.currentAccount
                    height: Kube.Units.gridUnit * 2
                    anchors {
                        top: border.bottom
                        left: statusBarContainer.left
                        right: statusBarContainer.right
                    }
                }
            }
        }

        WeekView {
            visible: weekViewButton.checked
            Layout.fillHeight: true
            Layout.fillWidth: true
            currentDate: root.currentDate
            startDate: DateUtils.getFirstDayOfWeek(root.selectedDate)
            calendarFilter: accountSwitcher.enabledCalendars
        }

        MonthView {
            visible: monthViewButton.checked
            Layout.fillHeight: true
            Layout.fillWidth: true
            currentDate: root.currentDate
            startDate: DateUtils.getFirstDayOfWeek(DateUtils.getFirstDayOfMonth(root.selectedDate))
            month: root.selectedDate.getMonth()
            calendarFilter: accountSwitcher.enabledCalendars
        }
    }

    Kube.Listener {
        filter: Kube.Messages.eventEditor
        onMessageReceived: eventPopup.createObject(root, message).open()
    }

    Component {
        id: eventPopup
        Kube.Popup {
            id: popup

            property alias start: editor.start
            property alias allDay: editor.allDay

            x: root.width * 0.15
            y: root.height * 0.15

            width: root.width * 0.7
            height: root.height * 0.7
            padding: 0
            EventEditor {
                id: editor
                anchors.fill: parent
                onDone: popup.close()
                accountId: accountSwitcher.currentAccount
            }
        }
    }
}
