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

ViewTestCase {
    id: testCase
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
        tryVerify(function(){ return newMailButton.activeFocus })
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
                    to: ["\"Quoted, Name\" <to@example.org>"],
                    cc: ["cc@example.org"],
                    bcc: ["bcc@example.org"],
                    draft: true
                }]
        }
        TestStore.setup(initialState)
        var composer = createTemporaryObject(composerComponent, testCase, {})

        var createdMail = TestStore.load("mail", {resource: "resource1"})

        composer.loadMessage(createdMail, Kube.ComposerController.Draft)

        var controller = findChild(composer, "composerController");
        verify(controller)
        tryVerify(function(){ return controller.accountId == "account1" })

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
                    body: "body\nnewline",
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

        var textEditor = findChild(composer, "textEditor");
        verify(textEditor)
        var expectedText = "you wrote:\n> body\n> newline"
        tryVerify(function(){ return textEditor.text.match(expectedText) })
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

    function test_5loadAccount() {
        var initialState = {
            accounts: [{
                    id: "account1",
                    id: "account2",
                }],
            identities: [
                {
                    account: "account1",
                    name: "Test Identity1",
                    address: "identity@example.org"
                },
                {
                    account: "account2",
                    name: "Test Identity",
                    address: "identity@example.org"
                }
            ],
            resources: [{
                    id: "resource1",
                    account: "account2",
                    type: "dummy"
                }]
        }
        TestStore.setup(initialState)

        var composer = createTemporaryObject(composerComponent, testCase, {accountId: "account2"})
        composer.setup()

        var controller = findChild(composer, "composerController");
        verify(controller)
        tryVerify(function(){ return controller.accountId == "account2" })

        var identityCombo = findChild(composer, "identityCombo");
        verify(identityCombo)
        compare(identityCombo.currentIndex, controller.identitySelector.currentIndex)
    }

    function test_6loadAccountNewMessage() {
        var initialState = {
            accounts: [{
                    id: "account1",
                    id: "account2",
                }],
            identities: [
                {
                    account: "account1",
                    name: "Test Identity1",
                    address: "identity@example.org"
                },
                {
                    account: "account2",
                    name: "Test Identity",
                    address: "identity@example.org"
                }
            ],
            resources: [{
                    id: "resource1",
                    account: "account2",
                    type: "dummy"
                }]
        }
        TestStore.setup(initialState)

        var composer = createTemporaryObject(composerComponent, testCase, {accountId: "account2", newMessage: true})
        composer.setup()

        var controller = findChild(composer, "composerController");
        verify(controller)
        tryVerify(function(){ return controller.accountId != "" })
    }

    function test_7noActionWithoutAccount() {
        var initialState = {
        }
        TestStore.setup(initialState)

        var composer = createTemporaryObject(composerComponent, testCase)
        composer.setup()

        var controller = findChild(composer, "composerController");

        //Without account we shouldn't be able to send or save as draft
        controller.accountId = ""
        verify(!controller.sendAction.enabled)
        controller.subject = "subject"
        controller.to.add({name: "test@example.org"})
        verify(!controller.sendAction.enabled)
        verify(!controller.saveAsDraftAction.enabled)

        //With account it should work though
        controller.accountId = "account1"
        verify(controller.sendAction.enabled)
        verify(controller.saveAsDraftAction.enabled)
    }

    function test_8saveReplyAsDraft() {
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
            folders: [{
                    id: "folder1",
                    resource: "resource1",
                    name: "Folder 1",
                    specialpurpose: ["inbox"],
                    mails: [{
                            resource: "resource1",
                            subject: "subject1",
                            body: "body",
                            to: ["to@example.org"],
                            draft: false
                        }
                    ],
                },
                {
                    id: "folder2",
                    resource: "resource1",
                    name: "Folder 2",
                    specialpurpose: ["drafts"],
                },
            ],
        }
        TestStore.setup(initialState)

        var createdMail = TestStore.load("mail", {resource: "resource1"})
        var composer = createTemporaryObject(composerComponent, testCase, {message: createdMail, loadType: Kube.ComposerController.Reply})
        composer.setup()

        tryVerify(function(){ return findChild(composer, "subject").text == "RE: subject1" })

        var textEditor = findChild(composer, "textEditor");
        verify(textEditor)
        tryVerify(function(){ return textEditor.text.match("body") })

        var controller = findChild(composer, "composerController");
        verify(controller)
        verify(controller.saveAsDraftAction.enabled)
        controller.saveAsDraftAction.execute()

        //Ensure draft message was created
        tryVerify(function(){ return TestStore.load("mail", {resource: "resource1", "draft": true}) })
        var draftMessage = TestStore.read(TestStore.load("mail", {resource: "resource1", "draft": true}))
        compare(draftMessage.subject, "RE: subject1")
        compare(draftMessage.draft, true)

        //Verify the original message is intact
        var m = TestStore.load("mail", {resource: "resource1", "draft": false})
        verify(m)
        var originalMail = TestStore.read(TestStore.load("mail", {resource: "resource1", "draft": false}))
        compare(originalMail.subject, "subject1")
        compare(originalMail.draft, false)
    }

    function test_9editDraft() {
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
                    draft: true
                }]
        }
        TestStore.setup(initialState)

        var createdMail = TestStore.load("mail", {resource: "resource1"})
        var composer = createTemporaryObject(composerComponent, testCase, {message: createdMail, loadType: Kube.ComposerController.Draft})
        composer.setup()

        //Edit message and save again
        var controller = findChild(composer, "composerController");
        verify(controller)
        tryVerify(function(){ return !controller.loading })

        findChild(composer, "subject").text = "subject2"

        verify(controller.saveAsDraftAction.enabled)
        controller.saveAsDraftAction.execute()

        //Ensure draft message was edited
        tryVerify(function(){ return TestStore.load("mail", {resource: "resource1", "subject": "subject2"}) })
        compare(TestStore.loadList("mail", {resource: "resource1"}).length, 1)
    }
}
