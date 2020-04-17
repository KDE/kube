/*
 *   Copyright 2020 Christian Mollekopf <mollekopf@kolabsys.com>
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
import QtQuick.Controls 2.0
import QtTest 1.0
import org.kube.test 1.0
import "../qml"
import "../applicationstate.js" as State

import org.kube.components.kube 1.0 as KubeComponent



TestCase {
    id: testCase
    width: app.width
    height: app.height
    name: "Screenshots"
    when: windowShown

    KubeComponent.Kube {
        id: app

        width: 1024
        height: 768

        Component.onCompleted: {
            TestStore.setup(State.initialState())
        }
    }

    function test_start() {
        //Wait for animations to settle
        wait(200)
        // grabToImage(function(result) {
        //     result.saveToFile("something.png");
        // });
        var image = grabImage(app.contentItem)
        // console.warn("Image ", image.size, app.contentItem.height)
        image.save("something.png")
        // wait(9999999)
    }
}
