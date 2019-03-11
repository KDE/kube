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
import QtQuick.Controls 2

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    property string text: document.text
    property bool htmlEnabled: document.containsFormatting

    property alias bold: document.bold
    property alias italic: document.italic
    property alias underline: document.underline
    property alias border: rect.border

    property alias font: edit.font
    property string initialText
    onInitialTextChanged: {
        edit.text = initialText
    }

    function clearFormatting() {
        document.resetFormat()
    }

    Kube.TextDocumentHandler {
        id: document
        document: edit.textDocument
        selectionStart: edit.selectionStart
        selectionEnd: edit.selectionEnd
        cursorPosition: edit.cursorPosition
    }

    Rectangle {
        id: rect
        anchors.fill: parent
        border.width: 1
        border.color: root.activeFocus ? Kube.Colors.highlightColor : Kube.Colors.buttonColor
        color: Kube.Colors.viewBackgroundColor

        Flickable {
            id: flickableItem
            anchors.fill: parent
            ScrollBar.vertical: Kube.ScrollBar {}
            clip: true

            Kube.ScrollHelper {
                anchors.fill: parent
                flickable: flickableItem
            }

            contentWidth: Math.max(edit.contentWidth, width)
            contentHeight: Math.max(edit.implicitHeight, height)

            function ensureVisible(r) {
                if (contentX >= r.x) {
                    contentX = r.x
                } else if (contentX+width <= r.x+r.width) {
                    contentX = r.x+r.width-width;
                }
                if (contentY >= r.y) {
                    contentY = r.y;
                } else if (contentY+height <= r.y+r.height) {
                    contentY = r.y+r.height-height;
                }
            }


            TextEdit {
                id: edit

                width: flickableItem.contentWidth
                height: flickableItem.contentHeight

                focus: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                textFormat: Qt.AutoText
                onCursorRectangleChanged: flickableItem.ensureVisible(cursorRectangle)

                color: Kube.Colors.textColor
                font.family: Kube.Font.fontFamily
                selectionColor: Kube.Colors.highlightColor
                padding: Kube.Units.smallSpacing
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                    cursorShape: Qt.IBeamCursor
                }
            }
        }
    }
}
