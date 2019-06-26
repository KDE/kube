/*
  Copyright (C) 2019 Christian Mollekopf, <mollekopf@kolabsys.com>

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

import QtQuick 2.7
import QtQuick.Controls 2
import QtQuick.Templates 2.0 as T

import org.kube.framework 1.0 as Kube

T.TextArea {
    id: root

    implicitWidth: contentWidth
    implicitHeight: contentHeight

    selectionColor: Kube.Colors.highlightColor
    readOnly: true
    selectByMouse: true

    wrapMode: TextEdit.Wrap
    textFormat: Text.AutoText

    font.family: Kube.Font.fontFamily
    color: Kube.Colors.textColor
    onLinkActivated: Qt.openUrlExternally(link)

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}
