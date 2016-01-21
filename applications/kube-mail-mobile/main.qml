/*
 * Copyright (C) 2015 Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

ApplicationWindow {
    id: app

    //FIXME remove fixed pixel hight
    //for now just convinience during testing
    width: 1080 / 2.7
    height: (1920 - 40)/ 2.7

    visible: true

    StackView {
        id: stack

        anchors.fill: parent

        //TODO set sink folderId property
        initialItem: {"item": Qt.resolvedUrl("FolderListView.qml"),properties: {stack: stack}}
    }

    //FIXME use whatever the plasma standart is
    Label {
        id: unit

        text: " "
    }

    ColorPalette {
        id: colorPalette
    }
}