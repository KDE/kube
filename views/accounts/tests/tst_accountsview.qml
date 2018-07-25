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
import org.kube.test 1.0
import "../qml"

TestCase {
    id: testCase
    width: 400
    height: 400
    name: "AccountsView"
    when: windowShown

    function cleanup() {
        TestStore.shutdownResources();
    }

    function visitChildren(item) {
        console.warn(item)
        for (var i = 0; i < item.children.length; i++) {
            visitChildren(item.children[i])
        }
    }

    Component {
        id: accountsComponent
        View {
        }
    }

    function test_1start() {
        var accountsView = createTemporaryObject(accountsComponent, testCase, {})
        verify(accountsView)
    }

    function test_2createAccount() {
        var accountsView = createTemporaryObject(accountsComponent, testCase, {})
        verify(accountsView)

        var accountWizard = findChild(accountsView, "accountWizard");
        verify(accountWizard)

        var typeButton = findChild(accountWizard.contentItem, "accountTypeButton" + "kolabnow")
        verify(typeButton)
        typeButton.clicked()

        var name = findChild(accountWizard.contentItem, "nameTextField")
        verify(name)
        name.text = "Name"

        var email = findChild(accountWizard.contentItem, "emailTextField")
        verify(email)
        email.text = "email@test.com"

        var save = findChild(accountWizard.contentItem, "saveButton")
        verify(save)
        save.clicked()

        var accounts = TestStore.loadList("account", {})
        compare(accounts.length, 1)
        var resources = TestStore.loadList("resource", {})
        compare(resources.length, 4)
    }
}
