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

FocusScope {
    id: root
    property alias model: repeater.model
    property alias delegate: repeater.delegate
    property alias contentHeight: flickable.contentHeight
    property int currentIndex: -1

    //We want to avoid interfering with scrolling as soon as the user starts to scroll. This is important if i.e. an html mail loads slowly.
    //However, we have to maintain position as the initial items expand, so we have to react to contentHeight changes. scrollToEnd ensures both.
    property bool scrollToEnd: true

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

    function incrementCurrentIndex() {
        flickable.incrementCurrentIndex()
    }

    function decrementCurrentIndex() {
        flickable.decrementCurrentIndex()
    }

    Flickable {
        id: flickable
        anchors.fill: parent

        //Optimize for view quality
        pixelAligned: true

        contentWidth: width
        contentHeight: col.height

        function scrollToIndex(index) {
            var item = repeater.itemAt(index)
            if (item) {
                scrollToPos(item.y)
            }
        }

        function scrollToPos(pos) {
            var scrollToEndPos = (flickable.contentHeight - flickable.height)
            //Avoid scrolling past the end
            if (pos < scrollToEndPos) {
                flickable.contentY = pos
            } else {
                flickable.contentY = scrollToEndPos
            }
        }

        onMovementStarted: {
            root.scrollToEnd = false
        }

        onContentHeightChanged: {
            if (repeater.count && root.scrollToEnd) {
                //Scroll to the last item
                root.currentIndex = repeater.count - 1
                flickable.scrollToIndex(root.currentIndex)
            }
        }

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
                    root.scrollToEnd = true
                    flickable.scrollToIndex(root.currentIndex)
                }
            }
        }

        function incrementCurrentIndex() {
            if (currentIndex < repeater.count - 1) {
                currentIndex = currentIndex + 1
            }
            scrollToIndex(currentIndex)
        }

        function decrementCurrentIndex() {
            if (currentIndex > 0) {
                currentIndex = currentIndex - 1
            }
            scrollToIndex(currentIndex)
        }

        Keys.onPressed: {
            if (event.matches(StandardKey.MoveToNextLine)) {
                scrollHelper.scrollDown()
            } else if (event.matches(StandardKey.MoveToPreviousLine)) {
                scrollHelper.scrollUp()
            }
        }

        Kube.ScrollHelper {
            id: scrollHelper
            flickable: flickable
            anchors.fill: parent
        }

        //Intercept all scroll events,
        //necessary due to the webengineview
        Kube.MouseProxy {
            anchors.fill: parent
            target: scrollHelper
            forwardWheelEvents: true
        }

        ScrollBar.vertical: Kube.ScrollBar {}

    }
}
