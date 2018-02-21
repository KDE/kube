/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

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

import org.kube.framework 1.0 as Kube

Item {
    id: root

    property string content
    property bool embedded: true
    property string type

    property string searchString
    property int contentHeight: textEdit.height

    onSearchStringChanged: {
        //This is a workaround because otherwise the view will not take the ViewHighlighter changes into account.
        textEdit.text = root.content
    }

    TextEdit {
        id: textEdit

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        selectionColor: Kube.Colors.highlightColor

        readOnly: true
        selectByMouse: true

        text: content
        wrapMode: TextEdit.Wrap
        textFormat: Text.RichText

        font.family: Kube.Font.fontFamily
        color: embedded ? Kube.Colors.disabledTextColor : Kube.Colors.textColor
        onLinkActivated: Qt.openUrlExternally(link)

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
        }
        Kube.ViewHighlighter {
            textDocument: textEdit.textDocument
            searchString: root.searchString
        }
    }
}
