/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


import QtQuick 2.7
import QtQuick.Controls 1.3
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1
import org.kube.framework 1.0 as Kube

FocusScope {
    id: container

    property int visibleViews: 2
    property int currentIndex: 0
    property int count: contentItems.length
    default property alias contentItems: content.data

    onCurrentIndexChanged: showRelevantSplits()
    Component.onCompleted: showRelevantSplits()

    function incrementCurrentIndex() {
        if (currentIndex < count) {
            currentIndex = currentIndex + 1
        }
    }

    function decrementCurrentIndex() {
        if (currentIndex > 0) {
            currentIndex = currentIndex - 1
        }
    }

    function showRelevantSplits() {
        var i;
        for (i = 0; i < count; i++) {
            if (i < currentIndex) {
                contentItems[i].visible = false;
            } else if (i > (currentIndex + visibleViews - 1)) {
                contentItems[i].visible = false;
            } else {
                contentItems[i].visible = true;
            }
        }

    }

    Kube.IconButton {
        anchors {
            top: container.top
            left: container.left
        }
        z: 1
        background: Rectangle {
            anchors.fill: parent
            color: Kube.Colors.textColor
        }
        iconName: Kube.Icons.goBack_inverted
        visible: currentIndex > 0
        onClicked: decrementCurrentIndex()
    }

    Keys.onEscapePressed: {
        if (currentIndex > 0) {
            decrementCurrentIndex()
        }
    }

    RowLayout {
        id: content
        anchors.fill: parent
    }
}
