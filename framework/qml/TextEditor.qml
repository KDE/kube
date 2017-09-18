/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import QtQuick.Controls 2.2

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    property string text: ""

    property bool htmlEnabled: false

    property alias bold: document.bold
    property alias italic: document.italic
    property alias underline: document.underline

    property string initialText
    onInitialTextChanged: {
        if (text == "") {
            edit.text = initialText
        }
    }

    Kube.DocumentHandler {
        id: document
        document: edit.textDocument
        selectionStart: edit.selectionStart
        selectionEnd: edit.selectionEnd
        //textColor: colorDialog.color
        onTextChanged: root.htmlEnabled ? root.text = htmlText : root.text = plainText

        cursorPosition: edit.cursorPosition
    }

    Kube.ScrollHelper {
        anchors.fill: parent
        flickable: flickableItem
        Flickable {
            id: flickableItem
            anchors.fill: parent
            ScrollBar.vertical: Kube.ScrollBar {}

            Kube.TextArea {
                id: edit
                focus: true
                anchors.fill: parent
                selectByMouse: true
                wrapMode: TextEdit.Wrap
            }
            TextArea.flickable: edit
        }
    }
}
