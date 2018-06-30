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

    Kube.TextEditor {
        id: editor
        initialText: "Foobar\nBarBar"
        htmlEnabled: false
    }

    function test_1initialText() {
        compare(editor.text, editor.initialText)
    }

    function test_2htmlConversion() {
        editor.htmlEnabled = true
        verify(editor.text.indexOf("<html>") !== -1)
        //It's converted into two paragraphs, so we can't check as a single string
        verify(editor.text.indexOf("Foobar") !== -1)
        verify(editor.text.indexOf("BarBar") !== -1)
        editor.htmlEnabled = false
        compare(editor.text, editor.initialText)
    }
}
