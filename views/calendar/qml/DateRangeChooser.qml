/*
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

import QtQuick 2
import QtQuick.Layouts 1
import QtQuick.Controls 2
import org.kube.framework 1.0 as Kube


import "dateutils.js" as DateUtils

ColumnLayout {
    id: root

    property date initialStart
    onInitialStartChanged: {
        startDate.dateTime = initialStart
    }
    property date initialEnd
    onInitialEndChanged: {
        endDate.dateTime = initialEnd
    }
    property alias allDay: checkBox.checked
    property date start: initialStart
    property date end: initialEnd

    property var notBefore: new Date(0)
    spacing: Kube.Units.smallSpacing

    Kube.Button {
        id: button

        Layout.preferredWidth: implicitWidth

        function formatString(start, end, allDay) {
            var startDate = start.toLocaleDateString();
            if (allDay) {
                if (DateUtils.sameDay(start, end)) {
                    // Tuesday, April 4, 2023
                    return startDate;
                } else {
                    // Tuesday, April 4, 2023 - Wednesday, April 12, 2023
                    return startDate + " " +
                        " - " +
                        end.toLocaleDateString();
                }
            }
            if (DateUtils.sameDay(start, end)) {
                // Tuesday, April 4, 2023 12:30 - 13:00
                return startDate + " " +
                    start.toLocaleTimeString(Qt.locale(), "hh:mm") +
                    " - " +
                    end.toLocaleTimeString(Qt.locale(), "hh:mm");
            }
            // Tuesday, April 4, 2023 12:30 - Wednesday, April 12, 2023 13:00
            return startDate + " " +
                start.toLocaleTimeString(Qt.locale(), "hh:mm") +
                " - " +
                end.toLocaleDateString() + " " +
                end.toLocaleTimeString(Qt.locale(), "hh:mm");
        }

        text: formatString(root.start, root.end, root.allDay)

        onClicked: {
            popup.open()
        }

        Kube.Popup {
            id: popup

            x: button.x
            y: button.y
            width: Math.max(selector.implicitWidth + Kube.Units.largeSpacing * 7, button.width)
            height: buttonOverlay.height + layout.implicitHeight + 2 * Kube.Units.smallSpacing
            modal: false
            focus: true
            padding: 0
            background: Rectangle {
                anchors.fill: parent
                color: Kube.Colors.viewBackgroundColor
                border.color: Kube.Colors.buttonColor
                border.width: 1
            }
            Kube.Button {
                id: buttonOverlay
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                height: button.height
                text: button.formatString(root.start, root.end, root.allDay)
                onClicked: popup.close()
            }

            ColumnLayout {
                id: layout
                anchors {
                    top: buttonOverlay.bottom
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                    topMargin: Kube.Units.smallSpacing
                    leftMargin: Kube.Units.smallSpacing
                    rightMargin: Kube.Units.smallSpacing
                    bottomMargin: Kube.Units.smallSpacing
                }

                DateSelector {
                    id: selector
                    notBefore: root.notBefore
                    backgroundColor: Kube.Colors.backgroundColor
                    textColor: Kube.Colors.textColor
                    invertIcons: false
                    rangeSelection: true
                    selectedDate: root.start
                    selectedEnd: root.end
                    onSelected: root.start = date
                    onEndSelected: root.end = date
                    onNext: root.start = DateUtils.nextMonth(selectedDate)
                    onPrevious: root.start = DateUtils.previousMonth(selectedDate)
                }

                RowLayout {
                    spacing: Kube.Units.smallSpacing
                    Kube.CheckBox {
                        id: checkBox
                    }
                    Kube.Label {
                        text: qsTr("All day")
                    }
                }

                RowLayout {
                    visible: !root.allDay
                    Kube.Label {
                        text: qsTr("begins")
                    }
                    TimeSelector {
                        id: startDate
                        Layout.preferredWidth: Kube.Units.gridUnit * 3
                        objectName: "startDate"
                        dateTime: root.start
                        onDateTimeChanged: {
                            root.start = dateTime
                        }
                    }
                    Kube.Label {
                        text: qsTr("ends")
                    }
                    TimeSelector {
                        id: endDate
                        Layout.preferredWidth: Kube.Units.gridUnit * 3
                        objectName: "endDate"
                        dateTime: root.end
                        notBefore: root.start
                        onDateTimeChanged: {
                            root.end = dateTime
                        }
                    }
                }
            }
        }
    }
}
