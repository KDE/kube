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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.2

import org.kde.kirigami 1.0 as Kirigami

Kirigami.Page {
    id: root

    anchors.fill: parent

    background: Rectangle {
        color: Kirigami.Theme.viewBackgroundColor
    }

    title: "Compose"

    ColumnLayout {
        anchors.fill: parent

        Controls.TextField {
            Layout.fillWidth: true

            placeholderText: "To"
        }

        Controls.TextField {
            Layout.fillWidth: true

            placeholderText: "Subject"
        }

        Controls.TextArea {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

}
