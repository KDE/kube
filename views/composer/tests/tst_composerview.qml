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
import org.kube.framework 1.0 as Kube
import org.kube.test 1.0
import "../qml"

TestCase {
    id: testCase
    width: 400
    height: 400
    name: "ComposerView"
    when: windowShown

    Component {
        id:composerComponent
        View {
            focus: true
        }
    }

    function test_1start() {
        var composer = createTemporaryObject(composerComponent, testCase, {})
        verify(composer)
    }

    function test_2verifyInitialFocus() {
        var composer = createTemporaryObject(composerComponent, testCase, {})
        var newMailButton = findChild(composer, "newMailButton");
        verify(newMailButton)
        verify(newMailButton.activeFocus)
    }

    function test_3sendMessage() {
        var initialState = {
            accounts: [{
                    id: "account1",
                }],
            identities: [{
                    account: "account1",
                    name: "Test Identity",
                    address: "identity@example.org"
                }],
            resources: [{
                    id: "resource1",
                    account: "account1",
                    type: "dummy"
                },
                {
                    id: "resource2",
                    account: "account1",
                    type: "mailtransport"
                }],
                mails:[{
                    resource: "resource1",
                    subject: "subject",
                    body: "body",
                    to: ["to@example.org"],
                    cc: ["cc@example.org"],
                    bcc: ["bcc@example.org"],
                    draft: true
                }]
        }
        TestStore.setup(initialState)
        var composer = createTemporaryObject(composerComponent, testCase, {})

        var createdMail = TestStore.load("mail", {resource: "resource1"})

        composer.loadMessage(createdMail, Kube.ComposerController.Draft)
        var sendMailButton = findChild(composer, "sendButton")
        verify(sendMailButton)
        tryVerify(function(){ return sendMailButton.enabled })
        sendMailButton.clicked()

        tryVerify(function(){ return TestStore.load("mail", {resource: "resource2"}) })
        tryVerify(function(){ return !TestStore.load("mail", {resource: "resource1"}) })

        var outgoingMail = TestStore.read(TestStore.load("mail", {resource: "resource2"}))
        compare(outgoingMail.subject, "subject")
        compare(outgoingMail.draft, false)
    }

    function test_4loadReply() {
        var initialState = {
            accounts: [{
                    id: "account1",
                }],
            identities: [{
                    account: "account1",
                    name: "Test Identity",
                    address: "identity@example.org"
                }],
            resources: [{
                    id: "resource1",
                    account: "account1",
                    type: "dummy"
                },
                {
                    id: "resource2",
                    account: "account1",
                    type: "mailtransport"
                }],
                mails:[{
                    resource: "resource1",
                    subject: "subject",
                    body: "body",
                    to: ["to@example.org"],
                    cc: ["cc@example.org"],
                    bcc: ["bcc@example.org"],
                }]
        }
        TestStore.setup(initialState)

        var createdMail = TestStore.load("mail", {resource: "resource1"})
        var composer = createTemporaryObject(composerComponent, testCase, {message: createdMail, loadType: Kube.ComposerController.Reply})
        composer.setup()

        var subject = findChild(composer, "subject");
        verify(subject)
        tryVerify(function(){ return subject.text == "RE: subject" })
        tryVerify(function(){ return subject.body != "" })
    }

    function test_5loadHtmlDraft() {
        var initialState = {
            accounts: [{
                    id: "account1",
                }],
            identities: [{
                    account: "account1",
                    name: "Test Identity",
                    address: "identity@example.org"
                }],
            resources: [{
                    id: "resource1",
                    account: "account1",
                    type: "dummy"
                },
                {
                    id: "resource2",
                    account: "account1",
                    type: "mailtransport"
                }],
                mails:[{
                    resource: "resource1",
                    subject: "subject",
                    body: "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'Noto Sans'; font-size:9pt; font-weight:400; font-style:normal;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">body</span></p></body></html>",
                    to: ["to@example.org"],
                    cc: ["cc@example.org"],
                    bcc: ["bcc@example.org"],
                }]
        }
        TestStore.setup(initialState)

        var createdMail = TestStore.load("mail", {resource: "resource1"})
        var composer = createTemporaryObject(composerComponent, testCase, {message: createdMail, loadType: Kube.ComposerController.Draft})
        composer.setup()

        var textEditor = findChild(composer, "textEditor");
        verify(textEditor)
        tryVerify(function(){ return textEditor.htmlEnabled == true })
    }
}
