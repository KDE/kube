/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

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

Item {
    id: root
    property alias rootIndex: visualModel.rootIndex
    property alias model: visualModel.model
    property alias searchString: visualModel.searchString
    property alias autoLoadImages: visualModel.autoLoadImages
    height: partListView.height

    ListView {
        id: partListView
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: contentHeight
        interactive: false
        model: MailPartModel {
            id: visualModel
        }
    }
}
