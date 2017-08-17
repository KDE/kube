/*
 *  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsystems.com>
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
import QtQuick.Controls 2
import QtQuick.Layouts 1.1
import org.kube.framework 1.0 as Kube

import QtQml 2.2 as QtQml

Flickable {
    id: root
    focus: true
    property alias model: repeater.model
    property alias delegate: repeater.delegate
    property int currentIndex: -1

    property var currentItem: null

    function setCurrentItem() {
        if (currentItem) {
            currentItem.isCurrentItem = false
        }
        if (currentIndex >= 0 && activeFocus) {
            var item = repeater.itemAt(currentIndex)
            if (item) {
                item.isCurrentItem = true
                currentItem = item
            }
        } else {
            currentItem = null
        }
    }

    onCurrentIndexChanged: {
        setCurrentItem()
    }

    onActiveFocusChanged: {
        setCurrentItem()
    }

    //Optimize for view quality
    pixelAligned: true

    contentWidth: width
    contentHeight: col.height

    function scrollToIndex(index) {
        var item = repeater.itemAt(index)
        var pos = item.y
        var scrollToEndPos = (root.contentHeight - root.height)
        //Avoid scrolling past the end
        if (pos < scrollToEndPos) {
            root.contentY = pos
        } else {
            root.contentY = scrollToEndPos
        }
    }

    onContentHeightChanged: {
        if (repeater.count) {
            //Scroll to the last item
            currentIndex = repeater.count - 1
            scrollToIndex(repeater.count - 1)
        }
    }

    property real span : contentY + height
    Column {
        id: col
        width: parent.width
        spacing: 2
        Repeater {
            id: repeater
            onCountChanged: {
                for (var i = 0; i < count; i++) {
                    itemAt(i).index = i
                }
            }
        }
    }

    function incrementCurrentIndex() {
        if (currentIndex < repeater.count - 1) {
            currentIndex = currentIndex + 1
        }
    }

    function decrementCurrentIndex() {
        if (currentIndex > 0) {
            currentIndex = currentIndex - 1
        }
    }

    Keys.onDownPressed: {
        incrementCurrentIndex()
        scrollToIndex(currentIndex)
    }

    Keys.onUpPressed: {
        decrementCurrentIndex()
        scrollToIndex(currentIndex)
    }

    Kube.ScrollHelper {
        id: scrollHelper
        flickable: root
        anchors.fill: parent
    }

    //Intercept all scroll events,
    //necessary due to the webengineview
    Kube.MouseProxy {
        anchors.fill: parent
        target: scrollHelper
        forwardWheelEvents: true
    }

    ScrollBar.vertical: ScrollBar {}

}
