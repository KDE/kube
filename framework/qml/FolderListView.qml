/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsystems.com>

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

import QtQuick 2.4

import org.kube.framework 1.0 as Kube

Kube.TreeView {
    id: root
    property variant accountId

    Kube.Listener {
        filter: Kube.Messages.selectNextFolder
        onMessageReceived: root.selectNext()
    }

    Kube.Listener {
        filter: Kube.Messages.selectPreviousFolder
        onMessageReceived: root.selectPrevious()
    }

    model: Kube.FolderListModel {
        id: folderListModel
        accountId: root.accountId
    }
}
