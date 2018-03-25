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
import org.kube.test 1.0

TestCase {
    id: testCase
    width: 400
    height: 400
    name: "MailView"

    Component {
        id: mailViewComponent
        View {
            focus: true
        }
    }

    function test_1start() {
        var mailView = createTemporaryObject(mailViewComponent, testCase, {})
        verify(mailView)
    }

    function test_2verifyInitialFocus() {
        var mailView = createTemporaryObject(mailViewComponent, testCase, {})
        var newMailButton = findChild(mailView, "newMailButton");
        verify(newMailButton)
        // verify(newMailButton.activeFocus)
    }

    function test_3selectMessage() {
        var initialState = {
            accounts: [{
                    id: "account1",
                    name: "Test Account"
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
                            cc: ["cc@example.org"],
                            bcc: ["bcc@example.org"],
                            draft: true
                        },
                        {
                            resource: "resource1",
                            subject: "subject2",
                            body: "body",
                            to: ["to@example.org"],
                            cc: ["cc@example.org"],
                            bcc: ["bcc@example.org"],
                            draft: true
                        }
                    ],
                }],
        }
        TestStore.setup(initialState)
        var mailView = createTemporaryObject(mailViewComponent, testCase, {})
        var folderListView = findChild(mailView, "folderListView");
        verify(folderListView)

        var folder = TestStore.load("folder", {resource: "resource1"})
        verify(folder)

        Kube.Fabric.postMessage(Kube.Messages.folderSelection, {"folder": folder, "trash": false});

        var mailListView = findChild(mailView, "mailListView");
        verify(mailListView)
        var listView = findChild(mailListView, "listView");
        verify(listView)
        tryCompare(listView, "count", 2)

        var conversationView = findChild(mailView, "mailView");
        verify(conversationView)
        var listView = findChild(conversationView, "listView");
        verify(listView)
    }

    function test_4moveToTrash() {
        var initialState = {
            accounts: [{
                    id: "account1",
                    name: "Test Account"
                }],
            identities: [{
                    account: "account1",
                    name: "Test Identity",
                    address: "identity@example.org"
                }],
            resources: [{
                    id: "resource3",
                    account: "account1",
                    type: "dummy"
                }],
            folders: [{
                    id: "folder1",
                    resource: "resource3",
                    name: "Folder 1",
                    specialpurpose: ["inbox"],
                    mails: [{
                            resource: "resource3",
                            subject: "subject1",
                            body: "body",
                            to: ["to@example.org"],
                            cc: ["cc@example.org"],
                            bcc: ["bcc@example.org"]
                        }
                    ],
                }],
        }
        TestStore.setup(initialState)
        var mailView = createTemporaryObject(mailViewComponent, testCase, {})
        var folderListView = findChild(mailView, "folderListView");
        verify(folderListView)

        var folder = TestStore.load("folder", {resource: "resource3"})
        verify(folder)

        Kube.Fabric.postMessage(Kube.Messages.folderSelection, {"folder": folder, "trash": false});

        var mailListView = findChild(mailView, "mailListView");
        verify(mailListView)
        var listView = findChild(mailListView, "listView");
        verify(listView)
        tryCompare(listView, "count", 1)

        listView.currentIndex = 0
        var currentItem = listView.currentItem
        verify(currentItem)

        var deleteButton = findChild(currentItem, "deleteButton");
        verify(deleteButton)
        deleteButton.clicked()

        tryCompare(listView, "count", 0)
    }
}
