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
import org.kube.framework 1.0 as Kube

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

    function test_1start() {
        verify(composer)
    }

    function test_2verifyInitialFocus() {
        var newMailButton = findChild(composer, "newMailButton");
        verify(newMailButton)
        verify(newMailButton.activeFocus)
    }

    Component {
        id: controllerComponent
        Kube.DomainObjectController {}
    }

    Component {
        id: outboxComponent
        Kube.OutboxModel {}
    }

    function test_3sendMessage() {
        var domainObjectController = controllerComponent.createObject(null, {blocking: true})
        var mail = {
            type: "mail",
            subject: "subject",
            body: "body",
            to: ["to@example.org"],
            cc: ["cc@example.org"],
            bcc: ["bcc@example.org"],
            draft: true
        }
        domainObjectController.create(mail)

        tryVerify(function(){ return domainObjectController.currentObject })
        var createdMail = domainObjectController.currentObject
        verify(createdMail)

        var loadAsDraft = true
        composer.loadMessage(createdMail, loadAsDraft)
        var sendMailButton = findChild(composer, "sendButton")
        verify(sendMailButton)
        tryVerify(function(){ return sendMailButton.enabled })
        sendMailButton.clicked()
        var outbox = outboxComponent.createObject(null, {})
        tryCompare(outbox, "count", 1)
    }
}
