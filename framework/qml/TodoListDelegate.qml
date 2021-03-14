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

Kube.GenericListDelegate {
    id: delegateRoot

    property var summary
    property var complete: false
    property var doing
    property var important: false
    property var calendar
    property var date
    property var dueDate
    property var domainObject

    property date currentDate

    mainText: summary
    subText: calendar
    disabled: complete
    strikeout: complete
    subtextVisible: hovered
    subtextDisabled: true

    onDropped: {
        if (dropAction == Qt.MoveAction) {
            Kube.Fabric.postMessage(Kube.Messages.moveToCalendar, {"todo": domainObject, "calendarId": dropTarget.calendarId})
        }
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
        const today = currentDate
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

    dateText: (!isNaN(dueDate) && !complete) ? formatDueDateTime(dueDate) : Qt.formatDateTime(date, "dd MMM yyyy")

    statusDelegate: Kube.Icon {
        iconName: Kube.Icons.isImportant
        visible:  delegateRoot.important
    }

    buttonDelegate: Column {
        Kube.IconButton {
            iconName: doing ? Kube.Icons.undo : Kube.Icons.addNew
            activeFocusOnTab: false
            tooltip: doing ? qsTr("Unpick") : qsTr("Pick")
            onClicked: {
                var controller = controllerComponent.createObject(parent, {"todo": domainObject});
                if (controller.complete) {
                    controller.complete = false
                }
                controller.doing = !controller.doing;
                controller.saveAction.execute();
            }
        }

        Kube.IconButton {
            iconName: Kube.Icons.checkbox
            checked: complete
            activeFocusOnTab: false
            tooltip: qsTr("Done!")
            onClicked: {
                var controller = controllerComponent.createObject(parent, {"todo": domainObject});
                controller.complete = !controller.complete;
                controller.saveAction.execute();
            }
        }
    }
}
