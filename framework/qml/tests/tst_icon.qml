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
    name: "Icon"

    Component {
        id: iconComponent
        Kube.Icon {}
    }

    function test_1iconSize() {
        var icon = createTemporaryObject(iconComponent, testCase, {iconName: Kube.Icons.mail_inverted, width: 48, height: 48})
        var image = findChild(icon, "image");
        verify(image)
        compare(image.status, Image.Ready)
        compare(image.paintedWidth, 22)
        compare(image.paintedHeight, 22)
    }

    function test_2iconSize() {
        var icon = createTemporaryObject(iconComponent, testCase, {iconName: Kube.Icons.overflowMenu_inverted, width: 24, height: 12})
        var image = findChild(icon, "image");
        verify(image)
        compare(image.status, Image.Ready)
        compare(image.paintedWidth, 24)
        compare(image.paintedHeight, 24)
    }

    function test_3iconSize() {
        var icon = createTemporaryObject(iconComponent, testCase, {iconName: Kube.Icons.overflowMenu_inverted, width: 48, height: 24})
        var image = findChild(icon, "image");
        verify(image)
        compare(image.status, Image.Ready)
        compare(image.paintedWidth, 32)
        compare(image.paintedHeight, 32)
    }
}
