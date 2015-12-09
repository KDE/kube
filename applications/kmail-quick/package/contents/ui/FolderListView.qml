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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.akonadi2.mail 1.0 as Mail

Item {
    id: root

    Item {
        id: searchBox

        width: root.width
        height: unit.size * 12

        TextField {
            anchors. centerIn: parent

            width: parent.width * 0.9

            placeholderText: "Search all email..."

        }
    }

    ScrollView {

        anchors {
            top: searchBox.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        ListView {
            id: listView

            clip: true

            model: folderList.model //FolderModel {}

            delegate: PlasmaComponents.ListItem {

                width: root.width
                height: unit.size * 10

                enabled: true

                onClicked: {
                    mailList.folderId.loadMailFolder(model.id)
                }

                PlasmaCore.IconItem {
                    id: iconItem

                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: unit.size * 3
                    }

                    source: model.icon
                }

                Label {
                    id: label

                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: iconItem.right
                        leftMargin: unit.size * 3
                    }

                    text: model.name
                }
            }
        }
    }
}