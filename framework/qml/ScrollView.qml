/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
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
import QtQuick 2.5
import QtQuick.Controls 2
import QtQuick.Controls 2.2 as Controls2
import org.kube.framework 1.0 as Kube

MouseArea {
    id: root
    default property Item contentItem
    property Flickable flickableItem
    property bool isMobile: false
    property bool scrolling: false

    onContentItemChanged: {
        if (contentItem.hasOwnProperty("contentY")) {
            //This already is a flickable
            flickableItem = contentItem;
            if (typeof(flickableItem.keyNavigationEnabled) != "undefined") {
                flickableItem.keyNavigationEnabled = true;
            }
            contentItem.parent = flickableParent;
        } else {
            //Create a flickable
            flickableItem = flickableComponent.createObject(flickableParent);
            contentItem.parent = flickableItem.contentItem;
        }
        //TODO: find a way to make flicking work on laptops with touch screen
        flickableItem.interactive = isMobile;
        flickableItem.anchors.fill = flickableParent;
        flickableItem.ScrollBar.vertical = scrollComponent.createObject(root);
        flickableItem.ScrollBar.vertical.anchors.right = root.right
        flickableItem.ScrollBar.vertical.anchors.top = root.top
        flickableItem.ScrollBar.vertical.anchors.bottom = root.bottom
        //FIXME Results in a "Cannot anchor to an item that is not a parent or a sibling" warning, but we need it to scroll the textfield
        contentItem.anchors.fill = flickableItem.contentItem;
    }

    Item {
        id: flickableParent
        anchors {
            fill: parent
        }
        clip: true
    }
    Component {
        id: flickableComponent
        Flickable {
            anchors {
                fill: parent
            }
            contentWidth: root.contentItem ? root.contentItem.width : 0
            contentHeight: root.contentItem ? root.contentItem.height : 0
        }
    }
    Component {
        id: scrollComponent
        ScrollBar {
            z: flickableParent.z + 1
            hoverEnabled: true
            active: root.scrolling || hovered
            onActiveChanged: {
                console.warn("ActiveChanged", active)
            }
            onVisibleChanged: {
                console.warn("Visible", visible)
            }
        }
    }

    drag.filterChildren: !isMobile
    onWheel: {
        if (isMobile || flickableItem.contentHeight < flickableItem.height) {
            return;
        }
        //Ignore 0 events (happens at least with Christians trackpad)
        if (wheel.pixelDelta.y == 0 && wheel.angleDelta.y == 0) {
            return;
        }
        //TODO somehow deal with the situation of getting 0 pixelDelta, but still getting an angleDelta every now and then.
        // var useAngle = wheel.pixelDelta.y != 0
        var useAngle = true
        var delta = wheel.pixelDelta.y

        var wheelScrollLines = 3
        //Try to get the size of one item in case of a list
        var sampleItem = flickableItem.itemAt ? flickableItem.itemAt(0, flickableItem.contentY) : null;
        //Otherwise just use a hardcoded value
        var oneLine = Kube.Units.gridUnit + Kube.Units.smallSpacing * 2;
        var lineSize = sampleItem ? sampleItem.height : oneLine;

        var step = Math.min(lineSize * wheelScrollLines, Kube.Units.gridUnit * 8);

        var y = useAngle ? delta : (wheel.angleDelta.y > 0 ? step : -step)
        if (!y) {
            return;
        }

        var minYExtent = flickableItem.topMargin;
        var maxYExtent = flickableItem.height - (flickableItem.contentHeight + flickableItem.bottomMargin + flickableItem.originY);

        if (typeof(flickableItem.headerItem) !== "undefined" && flickableItem.headerItem) {
            minYExtent += flickableItem.headerItem.height
        }

        flickableItem.contentY = Math.min(-maxYExtent, Math.max(-minYExtent, flickableItem.contentY - y));

        root.scrolling = true
        cancelFlickStateTimer.restart();
    }

    Timer {
        id: cancelFlickStateTimer
        //How long the scrollbar will remain visible
        interval: 500
        onTriggered: root.scrolling = false
    }
}
