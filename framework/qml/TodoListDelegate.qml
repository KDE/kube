/*
  Copyright (C) 2021 Christian Mollekopf, <christian@mkpf.ch>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.9
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.ListDelegate {
    id: delegateRoot
    property bool buttonsVisible: delegateRoot.hovered

    property var summary
    property var complete
    property var doing
    property var important
    property var calendar
    property var date
    property var dueDate
    property var domainObject
    property var color

    property var state

    height: Kube.Units.gridUnit * 3 + 2 * Kube.Units.smallSpacing

    color: Kube.Colors.viewBackgroundColor
    border.color: Kube.Colors.backgroundColor
    border.width: 1


    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        drag.target: parent
        drag.filterChildren: true
        onReleased: {
            if (parent.Drag.drop() == Qt.MoveAction) {
                Kube.Fabric.postMessage(Kube.Messages.moveToCalendar, {"todo": model.domainObject, "calendarId": parent.Drag.target.calendarId})
            }
        }
        onClicked: delegateRoot.clicked()
    }

    states: [
        State {
            name: "dnd"
            when: mouseArea.drag.active

            PropertyChanges {target: mouseArea; cursorShape: Qt.ClosedHandCursor}
            PropertyChanges {target: delegateRoot; opacity: 0.5}
            ParentChange {target: delegateRoot; parent: root; x: x; y: y}
        }
    ]

    Drag.active: mouseArea.drag.active
    Drag.hotSpot.x: mouseArea.mouseX
    Drag.hotSpot.y: mouseArea.mouseY
    Drag.source: delegateRoot

    Item {
        id: content

        anchors {
            fill: parent
            margins: Kube.Units.smallSpacing
        }

        Rectangle {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
            width: Kube.Units.smallSpacing
            height: width
            radius: width / 2

            color: model.color
        }
        Column {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin:  Kube.Units.largeSpacing
            }

            Kube.Label{
                width: content.width - Kube.Units.gridUnit * 3
                text: model.summary
                color: model.complete ? delegateRoot.disabledTextColor : delegateRoot.textColor
                font.strikeout: model.complete
                font.bold: model.doing && root.state != "doing"
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
            }
            Kube.Label {
                visible: delegateRoot.hovered
                text: model.calendar
                color: delegateRoot.disabledTextColor
                font.italic: true
                width: delegateRoot.width - Kube.Units.gridUnit * 3
                elide: Text.ElideRight
            }
        }

        Kube.Label {
            id: date
            anchors {
                right: parent.right
                bottom: parent.bottom
            }

            function sameDay(date1, date2) {
                return date1.getFullYear() == date2.getFullYear() && date1.getMonth() == date2.getMonth() && date1.getDate() == date2.getDate()
            }

            function daysSince(date1, date2) {
                //FIXME this is not going to work at month borders
                return (date1.getDate() - date2.getDate())
            }

            //TODO deal with start dates
            function formatDueDateTime(date) {
                const today = root.currentDate
                if (sameDay(date, today)) {
                    return qsTr("Due today")
                }
                const nextWeekToday = today.getTime() + ((24*60*60*1000) * 7);
                if (date.getTime() < nextWeekToday && date.getTime() > today.getTime()) {
                    return Qt.formatDateTime(date, "dddd") + qsTr(" (%1 days)").arg(daysSince(date, today))
                }
                if (date.getTime() < today.getTime()) {
                    return qsTr("Overdue for %1 days").arg(daysSince(today, date))
                }
                return Qt.formatDateTime(date, "dd MMM yyyy")
            }

            visible: !isNaN(model.date) && !delegateRoot.buttonsVisible
            text: (!isNaN(model.dueDate) && !model.complete) ? formatDueDateTime(model.dueDate) : Qt.formatDateTime(model.date, "dd MMM yyyy")
            font.italic: true
            color: delegateRoot.disabledTextColor
            font.pointSize: Kube.Units.tinyFontSize
        }
    }

    Kube.Icon {
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: Kube.Units.smallSpacing
        }

        visible:  model.important && !delegateRoot.buttonsVisible
        iconName: Kube.Icons.isImportant
    }

    Column {
        id: buttons

        anchors {
            right: parent.right
            margins: Kube.Units.smallSpacing
            verticalCenter: parent.verticalCenter
        }

        visible: delegateRoot.buttonsVisible
        opacity: 0.7

        Kube.IconButton {
            iconName: model.doing ? Kube.Icons.undo : Kube.Icons.addNew
            activeFocusOnTab: false
            tooltip: model.doing ? qsTr("Unpick") : qsTr("Pick")
            onClicked: {
                var controller = controllerComponent.createObject(parent, {"todo": model.domainObject});
                if (controller.complete) {
                    controller.complete = false
                }
                controller.doing = !controller.doing;
                controller.saveAction.execute();
            }
        }

        Kube.IconButton {
            iconName: Kube.Icons.checkbox
            checked: model.complete
            activeFocusOnTab: false
            tooltip: qsTr("Done!")
            onClicked: {
                var controller = controllerComponent.createObject(parent, {"todo": model.domainObject});
                controller.complete = !controller.complete;
                controller.saveAction.execute();
            }
        }

    }
}
