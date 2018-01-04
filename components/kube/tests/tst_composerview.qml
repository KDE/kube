/*
 *   Copyright 2017 Christian Mollekopf <mollekopf@kolabsys.com>
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
import "../qml"

TestCase {
    id: testCase
    width: 400
    height: 400
    name: "ComposerView"
    when: windowShown

    ComposerView {
        id: composer
        focus: true
    }

    function test_start() {
        verify(composer)
    }

    function test_verifyInitialFocus() {
        var newMailButton = findChild(composer, "newMailButton");
        verify(newMailButton)
        verify(newMailButton.activeFocus)
    }

    function test_sendMessage() {
        var mail = null
        var loadAsDraft = true
        composer.loadMessage(mail, loadAsDraft)
        var sendMailButton = findChild(composer, "sendButton")
        verify(sendMailButton)
        verify(sendMailButton.enabled)
        sendMailButton.clicked()
        //TODO verify the mail is sent
    }
}
