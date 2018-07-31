/*
 *   Copyright 2017 Christian Mollekopf <mollekopf@kolabsystems.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.7
import QtTest 1.0
import org.kube.framework 1.0 as Kube


TestCase {
    id: testCase
    width: 400
    height: 400
    name: "TextEditor"

    Component {
        id: editorComponent
        Kube.TextEditor {}
    }

    property string htmlText: "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'Noto Sans'; font-size:9pt; font-weight:400; font-style:normal;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><b>Foobar</b><br>BarBar</p></body></html>"
    property string plainText: "Foobar\nBarBar"

    function test_1initialText() {
        var editor = createTemporaryObject(editorComponent, testCase, {initialText: plainText})
        compare(editor.text, editor.initialText)
    }

    function test_3htmlToPlainConversion() {
        var editor = createTemporaryObject(editorComponent, testCase, {initialText: htmlText})
        editor.clearFormatting()
        compare(editor.text, plainText)
    }

    function test_4detectPlain() {
        var editor = createTemporaryObject(editorComponent, testCase, {initialText: plainText})
        compare(editor.htmlEnabled, false)
    }

    function test_5detectHtml() {
        var editor = createTemporaryObject(editorComponent, testCase, {initialText: htmlText})
        compare(editor.htmlEnabled, true)
    }
}
