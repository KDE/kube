/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Window 2.0

import QtQuick.Controls 2.0 as Controls2
import org.kube.framework 1.0 as Kube

Controls2.ApplicationWindow {
    id: app
    height: Screen.desktopAvailableHeight * 0.4
    width: Screen.desktopAvailableWidth * 0.4
    visible: true
    font.family: Kube.Font.fontFamily
    ColumnLayout  {
        anchors {
            centerIn: parent
            margins: Kube.Units.largeSpacing
        }
        spacing: 0
        Kube.Heading {
            text: qsTr("Please wait while Kube is upgrading...")
            color: Kube.Colors.highlightColor
        }
        Kube.Label {
            text: qsTr("This might take a while.")
        }
    }
}
