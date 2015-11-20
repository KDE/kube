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
    height: 1080 * 0.7
    width: 1920  * 0.7

    visible: true

    toolBar: ToolBar {

        Row {
            anchors.fill: parent

            ToolButton {
                iconName: "mail-receive"
            }

            ToolButton {
                iconName: "mail-message-new"
            }
        }
    }

    SplitView {
        anchors.fill: parent

        FolderListView {
            id: folderList

            width: unit.size * 55
            Layout.maximumWidth: unit.size * 150
            Layout.minimumWidth: unit.size * 30
        }

        MailListView  {
            id: mailList

            width: unit.size * 80
            Layout.maximumWidth: unit.size * 250
            Layout.minimumWidth: unit.size * 50
        }

        SingleMailView {
            id: mailView

            Layout.fillWidth: true
        }
    }

    //TODO find a better way to scale UI
    Item {
        id: unit

        property int size: 5
    }

    ColorPalette {
	id: colorPalette
    }
}

