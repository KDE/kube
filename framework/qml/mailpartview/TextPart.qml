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
import QtQuick.Controls 2

import org.kube.framework 1.0 as Kube

Item {
    id: root

    property string content
    property bool embedded: true
    property string type
    property bool autoLoadImages: false

    property string searchString
    property int contentHeight: textEdit.height

    onSearchStringChanged: {
        //This is a workaround because otherwise the view will not take the ViewHighlighter changes into account.
        textEdit.text = root.content
    }

    Kube.TextArea {
        id: textEdit

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        text: content.substring(0, 100000) //The TextEdit deals poorly with messages that are too large.
        color: embedded ? Kube.Colors.disabledTextColor : Kube.Colors.textColor

        Kube.ViewHighlighter {
            textDocument: textEdit.textDocument
            searchString: root.searchString
        }
    }
}
