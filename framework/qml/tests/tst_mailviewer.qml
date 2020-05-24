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
    name: "MailViewer"
    when: windowShown
    visible: true

    Kube.File {
        id: file
        path: "/src/kube/framework/qml/tests/kraken.eml"
    }

    Component {
        id: viewerComponent
        Kube.MailViewer {
            anchors.fill: parent
            visible: true
            unread: false
            trash: false
            draft: false
            sent: false
            incomplete: false
            current: true
            autoLoadImages: true
        }
    }

    function test_1htmlMail() {
        var viewer = createTemporaryObject(viewerComponent, testCase, {message: file.data})

        var htmlButton = findChild(viewer, "htmlButton");
        verify(htmlButton)
        tryVerify(function(){ return htmlButton.visible })
        htmlButton.clicked()

        var htmlView = findChild(viewer, "htmlView");
        verify(htmlView)
        tryVerify(function(){ return htmlView.loadProgress == 100})

        tryVerify(function(){ return (1300 > htmlView.width && htmlView.width > 1200)})
        tryVerify(function(){ return (2700 > htmlView.height && htmlView.height > 2400)})
    }

}
