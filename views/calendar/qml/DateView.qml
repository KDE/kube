/*
 *  Copyright (C) 2018 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2

import org.kube.framework 1.0 as Kube

Row {
    id: root
    property date date: null

    spacing: Kube.Units.smallSpacing

    Kube.Label {
        anchors.verticalCenter: parent.verticalCenter
        text: root.date.getDate()
        color: Kube.Colors.highlightedTextColor
        font.pointSize: Kube.Units.defaultFontSize * 3
    }
    Column {
        anchors.verticalCenter: parent.verticalCenter
        Kube.Label {
            text: root.date.toLocaleString(Qt.locale(), "dddd")
            color: Kube.Colors.highlightedTextColor
        }
        Kube.Label {
            text: root.date.toLocaleString(Qt.locale(), "MMMM yyyy")
            color: Kube.Colors.highlightedTextColor
            font.pointSize: Kube.Units.smallFontSize
        }
    }
}
