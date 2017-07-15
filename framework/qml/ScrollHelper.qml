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
import org.kube.framework 1.0 as Kube

/*
    * The MouseArea + interactive: false + maximumFlickVelocity are required
    * to fix scrolling for desktop systems where we don't want flicking behaviour.
    *
    * See also:
    * ScrollView.qml in qtquickcontrols
    * qquickwheelarea.cpp in qtquickcontrols
    */
MouseArea {
    id: root
    propagateComposedEvents: true

    property Flickable flickable

    //Place the mouse area under the flickable
    z: -1
    onFlickableChanged: {
        flickable.interactive = false
        flickable.maximumFlickVelocity = 100000
        flickable.boundsBehavior = Flickable.StopAtBounds
        root.parent = flickable
    }

    function calculateNewPosition(flickableItem, wheel) {
        //Nothing to scroll
        if (flickableItem.contentHeight < flickableItem.height) {
            return flickableItem.contentY;
        }
        //Ignore 0 events (happens at least with Christians trackpad)
        if (wheel.pixelDelta.y == 0 && wheel.angleDelta.y == 0) {
            return flickableItem.contentY;
        }
        //pixelDelta seems to be the same as angleDelta/8
        var pixelDelta = wheel.pixelDelta.y != 0 ? wheel.pixelDelta.y : wheel.angleDelta.y / 8

        //We're only doing pixed based scrolling for the time being
        //TODO somehow deal with the situation of getting 0 pixelDelta, but still getting an angleDelta every now and then.
        // var useAngle = wheel.pixelDelta.y != 0
        // var wheelScrollLines = 3
        // //Try to get the size of one item in case of a list
        // var sampleItem = flickableItem.itemAt ? flickableItem.itemAt(0, flickableItem.contentY) : null;
        // //Otherwise just use a hardcoded value
        // var oneLine = Kube.Units.gridUnit + Kube.Units.smallSpacing * 2;
        // var lineSize = sampleItem ? sampleItem.height : oneLine;

        // var step = Math.min(lineSize * wheelScrollLines, Kube.Units.gridUnit * 8);
        // var y = useAngle ? delta : (wheel.angleDelta.y > 0 ? step : -step)

        var y = pixelDelta
        if (!y) {
            return flickableItem.contentY;
        }

        var minYExtent = flickableItem.originY + flickableItem.topMargin;
        var maxYExtent = (flickableItem.contentHeight + flickableItem.bottomMargin + flickableItem.originY) - flickableItem.height;

        if (typeof(flickableItem.headerItem) !== "undefined" && flickableItem.headerItem) {
            minYExtent += flickableItem.headerItem.height
        }

        //Avoid overscrolling
        return Math.max(minYExtent, Math.min(maxYExtent, flickableItem.contentY - y));
    }

    onWheel: {
        //Some trackpads (mine) emit 0 events in between that we can safely ignore.
        // if (wheel.pixelDelta.y) {
        //     //120 is apparently the factor used in windows(https://chromium.googlesource.com/chromium/src/+/70763eb93a32555910a3b4269aeec51252ab9ec6/ui/events/event.cc)
        //     listView.flick(0, wheel.pixelDelta.y * 120)
        // } else if (wheel.angleDelta.y) {
        //     //Arbitrary but this seems to work for me...
        //     listView.flick(0, wheel.angleDelta.y * 10)
        // }

        // console.warn("Delta: ", wheel.angleDelta.y);
        // console.warn("Old position: ", flickable.contentY);
        // console.warn("New position: ", newPos);
        var newPos = calculateNewPosition(flickable, wheel);
        // Show the scrollbars
        flickable.flick(0, 0);
        flickable.contentY = newPos;
        cancelFlickStateTimer.start()
    }


    Timer {
        id: cancelFlickStateTimer
        //How long the scrollbar will remain visible
        interval: 500
        // Hide the scrollbars
        onTriggered: flickable.cancelFlick();
    }
}

