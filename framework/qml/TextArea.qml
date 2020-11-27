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
import QtQuick.Controls 2.13
import QtQuick.Templates 2.13 as T

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
    hoverEnabled: true
    onLinkActivated: {
        console.warn("Link activated", link)
        Qt.openUrlExternally(link)
    }

    //FIXME with qt 5.13 connecting to this signal is a requirement for the cursorShape to be changed
    onLinkHovered: {
        console.warn("Link hovered", link)
    }

    MouseArea {
        id: contextMenuMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        property string hoveredLink: null

        onPressed: {
            // selection gets lost due to focus loss
            var selectionStart = root.selectionStart
            var selectionEnd = root.selectionEnd
            if (root.linkHovered) {
                contextMenuMouseArea.hoveredLink = root.hoveredLink
            } else {
                contextMenuMouseArea.hoveredLink = null
            }
            contextMenu.popup()
            root.select(selectionStart, selectionEnd)
        }

        Menu {
            id: contextMenu
            MenuItem {
                text: contextMenuMouseArea.hoveredLink ? qsTr("Copy Link") : qsTr("Copy Selection")
                onTriggered: {
                    if (contextMenuMouseArea.hoveredLink) {
                        clipboard.text = contextMenuMouseArea.hoveredLink
                    } else {
                        root.copy()
                    }
                }
                enabled: root.selectedText.length > 0 || contextMenuMouseArea.hoveredLink
                Kube.Clipboard {
                    id: clipboard
                }
            }
        }
    }
}

