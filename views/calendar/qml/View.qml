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
import QtQuick.Controls 2
import QtQuick.Layouts 1.2

import org.kube.framework 1.0 as Kube
import "dateutils.js" as DateUtils


Kube.View {
    id: root

    property date currentDate: new Date()
    property date selectedDate: getFirstDayOfWeek(currentDate)
    property bool autoUpdateDate: true

    Kube.CheckedEntities {
        id: calendarFilterCollector
    }

    onSelectedDateChanged: {
        console.log("Selected date changed", selectedDate)
    }

    onRefresh: {
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "calendar"})
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "event"})
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

    function getFirstDayOfMonth(date) {
        var d = date
        d.setDate(1)
        return d
    }

    RowLayout {

        Timer {
            running: autoUpdateDate
            interval: 2000; repeat: true
            onTriggered: root.currentDate = new Date()
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
                    onClicked: {
                        eventPopup.open()
                    }
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
                        onCheckedChanged: {
                            if (checked) {
                                root.selectedDate = getFirstDayOfWeek(root.selectedDate)
                            }
                        }
                    }
                    Kube.TextButton {
                        id: monthViewButton
                        Layout.fillWidth: true
                        text: qsTr("Month")
                        textColor: Kube.Colors.highlightedTextColor
                        checkable: true
                        ButtonGroup.group: viewButtonGroup
                        onCheckedChanged: {
                            if (checked) {
                                root.selectedDate = getFirstDayOfMonth(root.selectedDate)
                            }
                        }
                    }
                }
                /*
                DateView {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    date: root.currentDate
                    MouseArea {
                        anchors.fill: parent
                        onClicked: dateSelector.selectedDate = root.currentDate
                    }
                }
                */

                DateSelector {
                    id: dateSelector
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    selectedDate: root.selectedDate
                    onSelected: {
                        if (weekViewButton.checked) {
                            root.selectedDate = getFirstDayOfWeek(date)
                        } else {
                            root.selectedDate = getFirstDayOfMonth(date)
                        }
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

            Kube.InlineAccountSwitcher {
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


                delegate: Kube.ListView {
                    id: listView
                    spacing: Kube.Units.smallSpacing
                    model: Kube.CheckableEntityModel {
                        id: calendarModel
                        type: "calendar"
                        roles: ["name", "color", "enabled"]
                        sortRole: "name"
                        accountId: listView.parent.accountId
                        checkedEntities: calendarFilterCollector
                    }
                    delegate: Kube.ListDelegate {
                        id: delegate
                        width: listView.availableWidth
                        height: Kube.Units.gridUnit
                        hoverEnabled: true
                        background: Item {}
                        RowLayout {
                            anchors.fill: parent
                            spacing: Kube.Units.smallSpacing
                            Kube.CheckBox {
                                id: checkBox
                                opacity: 0.9
                                checked: model.checked || model.enabled
                                onCheckedChanged: {
                                    model.checked = checked
                                    model.enabled = checked
                                }

                                indicator: Rectangle {
                                    width: Kube.Units.gridUnit
                                    height: Kube.Units.gridUnit

                                    color: model.color

                                    Rectangle {
                                        id: highlight
                                        anchors.fill: parent
                                        visible: checkBox.hovered || checkBox.visualFocus
                                        color: Kube.Colors.highlightColor
                                        opacity: 0.4
                                    }

                                    Kube.Icon {
                                        anchors.centerIn: parent
                                        height: Kube.Units.gridUnit
                                        width: Kube.Units.gridUnit
                                        visible: checkBox.checked
                                        iconName: Kube.Icons.checkbox_inverted
                                    }
                                }

                            }
                            Kube.Label {
                                id: label
                                Layout.fillWidth: true
                                text: model.name
                                color: Kube.Colors.highlightedTextColor
                                elide: Text.ElideLeft
                                clip: true
                            }
                            ToolTip {
                                id: toolTipItem
                                visible: delegate.hovered && label.truncated
                                text: label.text
                            }
                        }
                    }
                }
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
            startDate: root.selectedDate
            calendarFilter: calendarFilterCollector.checkedEntities
        }

        MonthView {
            visible: monthViewButton.checked
            Layout.fillHeight: true
            Layout.fillWidth: true
            currentDate: root.currentDate
            startDate: getFirstDayOfWeek(getFirstDayOfMonth(root.selectedDate))
            month: root.selectedDate.getMonth()
            calendarFilter: calendarFilterCollector.checkedEntities
        }
    }

    EventEditor {
        id: eventPopup

        x: root.width * 0.15
        y: root.height * 0.15

        width: root.width * 0.7
        height: root.height * 0.7
    }
}
