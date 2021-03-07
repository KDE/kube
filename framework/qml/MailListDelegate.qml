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

import org.kube.framework 1.0 as Kube

Kube.GenericListDelegate {
    id: delegateRoot

    property var subject
    property var unread
    property var senderName
    property var date
    property var trash
    property var threadSize

    //Required for D&D
    property var mail

    mainText: subject
    subText: senderName
    active: unread
    disabled: false
    strikeout: false
    counter: threadSize
    hideSubtext: false

    onDropped: {
        if (dropAction == Qt.MoveAction) {
            delegateRoot.visible = false
        }
    }

    height: Kube.Units.gridUnit * 5

    function sameDay(date1, date2) {
        return date1.getFullYear() == date2.getFullYear() && date1.getMonth() == date2.getMonth() && date1.getDate() == date2.getDate()
    }

    function formatDateTime(date) {
        const today = new Date()
        if (sameDay(date, today)) {
            return Qt.formatDateTime(date, "hh:mm")
        }
        const lastWeekToday = today.getTime() - ((24*60*60*1000) * 7);
        if (date.getTime() >= lastWeekToday) {
            return Qt.formatDateTime(date, "ddd hh:mm")
        }
        return Qt.formatDateTime(date, "dd MMM yyyy")
    }

    dateText: formatDateTime(delegateRoot.date)

    buttons: [
       Kube.IconButton {
           id: restoreButton
           iconName: Kube.Icons.undo
           visible: !!delegateRoot.trash
           onClicked: Kube.Fabric.postMessage(Kube.Messages.restoreFromTrash, {"mail": delegateRoot.mail})
           activeFocusOnTab: false
           tooltip: qsTr("Restore from trash")
       },

       Kube.IconButton {
           id: readButton
           iconName: Kube.Icons.markAsRead
           visible: delegateRoot.unread && !delegateRoot.trash
           onClicked: Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": delegateRoot.mail})
           tooltip: qsTr("Mark as read")
       },

       Kube.IconButton {
           id: unreadButton
           iconName: Kube.Icons.markAsUnread
           visible: !delegateRoot.unread && !delegateRoot.trash
           onClicked: Kube.Fabric.postMessage(Kube.Messages.markAsUnread, {"mail": delegateRoot.mail})
           activeFocusOnTab: false
           tooltip: qsTr("Mark as unread")
       },

       Kube.IconButton {
           id: importantButton
           iconName: delegateRoot.important ? Kube.Icons.markImportant : Kube.Icons.markUnimportant
           visible: !!delegateRoot.mail
           onClicked: Kube.Fabric.postMessage(Kube.Messages.setImportant, {"mail": delegateRoot.mail, "important": !delegateRoot.important})
           activeFocusOnTab: false
           tooltip: qsTr("Mark as important")
       },

       Kube.IconButton {
           id: deleteButton
           objectName: "deleteButton"
           iconName: Kube.Icons.moveToTrash
           visible: !!delegateRoot.mail
           onClicked: Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": delegateRoot.mail})
           activeFocusOnTab: false
           tooltip: qsTr("Move to trash")
       }
   ]
}
