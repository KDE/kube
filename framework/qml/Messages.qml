/*
  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>

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

pragma Singleton

import QtQuick 2.7

Item {
    //Selections
    property string folderSelection: "currentFolder"
    property string mailSelection: "currentMail"
    property string accountSelection: "currentAccount"

    //Actions
    property string moveToTrash: "moveToTrash"
    property string restoreFromTrash: "restoreFromTrash"
    property string markAsRead: "markAsRead"
    property string markAsUnread: "markAsUnread"
    property string setImportant: "setImportant"
    property string moveToFolder: "moveToFolder"
    property string moveToCalendar: "moveToCalendar"
    property string moveToDrafts: "moveToDrafts"
    property string unlockKeyring: "unlockKeyring"
    property string requestLogin: "requestLogin"
    property string requestAccountsConfiguration: "requestAccountsConfiguration"

    property string notification: "notification"
    property string displayNotification: "displayNotification"
    property string progressNotification: "progressNotification"
    property string errorNotification: "errorNotification"
    property string search: "search"
    property string searchString: "searchString"
    property string synchronize: "synchronize"
    property string abortSynchronization: "abortSynchronization"
    property string reply: "reply"
    property string forward: "forward"
    property string edit: "edit"
    property string compose: "compose"
    property string sendOutbox: "sendOutbox"

    property string componentDone: "done"
    property string errorPending: "errorPending"
    property string notificationPending: "notificationPending"

    property string selectTopConversation: "selectTopConversation"
    property string selectNextConversation: "selectNextConversation"
    property string selectPreviousConversation: "selectPreviousConversation"
    property string selectNextMessage: "selectNextMessage"
    property string selectPreviousMessage: "selectPreviousMessage"
    property string selectNextFolder: "selectNextFolder"
    property string selectPreviousFolder: "selectPreviousFolder"
    property string scrollConversationDown: "scrollConversationDown"
    property string scrollConversationUp: "scrollConversationUp"

    property string eventEditor: "eventEditor"
}

