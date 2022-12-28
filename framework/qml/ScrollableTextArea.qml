/*
 *  Copyright (C) 2022 Christian Mollekopf, <christian@mkpf.ch>
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

import QtQuick 2.4
import QtQuick.Controls 2.3

import org.kube.framework 1.0 as Kube

Flickable {
    id: flickable

    property alias text: textArea.text
    property alias textFormat: textArea.textFormat
    property alias implicitHeight: textArea.implicitHeight

    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.horizontal: Kube.ScrollBar {  }
    contentHeight: textArea.height
    contentWidth: textArea.width
    clip: true
    Kube.TextArea {
        id: textArea
        width: flickable.width
    }
    Kube.ScrollHelper {
        anchors.fill: parent
        flickable: flickable
    }
}
