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

    //Actions
    property string moveToTrash: "moveToTrash"
    property string restoreFromTrash: "restoreFromTrash"
    property string markAsRead: "markAsRead"
    property string markAsUnread: "markAsUnread"
    property string toggleImportant: "toggleImportant"
    property string moveToFolder: "moveToFolder"
    property string moveToDrafts: "moveToDrafts"

    property string notification: "notification"
    property string progressNotification: "progressNotification"
    property string search: "search"
    property string synchronize: "synchronize"
    property string reply: "reply"
    property string edit: "edit"
    property string compose: "compose"

    property string componentDone: "done"
}

