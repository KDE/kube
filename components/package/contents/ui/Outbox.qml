/*
 * Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.components 1.0 as KubeComponents

ToolButton {
    id: root

    iconName: "mail-folder-outbox"

    onClicked: {
        dialog.visible = dialog.visible ? false : true
    }

    Rectangle {
        id: dialog

        anchors {
            top: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        height: 600
        width: 400

        color: Kirigami.Theme.backgroundColor
        border.width: 1
        border.color: Kirigami.Theme.highlightColor //TODO change to Kirigami inactive text color once it is available
        radius: 3
        clip: true
        visible: false
    }
}
