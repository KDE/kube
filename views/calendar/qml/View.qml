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

    property date selectedDate: Kube.Context.currentDate

    onRefresh: {
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "calendar"})
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "event"})
    }

    helpViewComponent: Kube.HelpPopup {
        ListModel {
            ListElement { description: qsTr("Jump to next week/month:"); shortcut: "j" }
            ListElement { description: qsTr("Jump to previous week/month:"); shortcut: "k" }
        }
    }

    Shortcut {
        enabled: root.isCurrentView
        sequences: ['j', StandardKey.Forward]
        onActivated: root.goToNext()
    }

    Shortcut {
        enabled: root.isCurrentView
        sequences: ['k', StandardKey.Back]
        onActivated: root.goToPrevious()
    }

    function goToNext() {
        if (weekViewButton.checked) {
            root.selectedDate = DateUtils.nextWeek(root.selectedDate)
        } else {
            root.selectedDate = DateUtils.nextMonth(root.selectedDate)
        }
    }
    function goToPrevious() {
        if (weekViewButton.checked) {
            root.selectedDate = DateUtils.previousWeek(root.selectedDate)
        } else {
            root.selectedDate = DateUtils.previousMonth(root.selectedDate)
        }
    }
    RowLayout {
        Kube.LeftSidebar {
            Layout.fillHeight: parent.height
            buttons: [
               Kube.PositiveButton {
                   id: newEventButton
                   objectName: "newEventButton"

                   Layout.fillWidth: true
                   focus: true
                   text: qsTr("New Event")
                   onClicked: eventPopup.createObject(root, {start: DateUtils.sameDay(Kube.Context.currentDate, root.selectedDate) ? Kube.Context.currentDate : root.selectedDate}).open()
               },
               RowLayout {
                   Layout.fillWidth: true
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
               },
               DateView {
                   Layout.fillWidth: true
                   date: Kube.Context.currentDate
                   MouseArea {
                       anchors.fill: parent
                       onClicked: {
                           root.selectedDate = Kube.Context.currentDate
                       }
                   }
               },
               DateSelector {
                   id: dateSelector
                   Layout.fillWidth: true
                   selectedDate: root.selectedDate
                   onSelected: {
                       root.selectedDate = date
                   }
                   onNext: root.goToNext()
                   onPrevious: root.goToPrevious()
               }
            ]

            Kube.CalendarSelector {
                id: accountSwitcher
                objectName: "calendarSelector"
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentType: "event"
            }
        }

        WeekView {
            visible: weekViewButton.checked
            Layout.fillHeight: true
            Layout.fillWidth: true
            currentDate: Kube.Context.currentDate
            startDate: DateUtils.getFirstDayOfWeek(root.selectedDate)
            calendarFilter: accountSwitcher.enabledEntities
        }

        MonthView {
            visible: monthViewButton.checked
            Layout.fillHeight: true
            Layout.fillWidth: true
            currentDate: Kube.Context.currentDate
            startDate: DateUtils.getFirstDayOfWeek(DateUtils.getFirstDayOfMonth(root.selectedDate))
            month: root.selectedDate.getMonth()
            calendarFilter: accountSwitcher.enabledEntities
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
                accountId: Kube.Context.currentAccountId
            }
        }
    }
}
