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
            contentItem.parent = flickableParent;
        }
        //TODO: find a way to make flicking work on laptops with touch screen
        flickableItem.interactive = isMobile;
        flickableItem.anchors.fill = flickableParent;
        flickableItem.ScrollBar.vertical = scrollComponent.createObject(root);
        flickableItem.ScrollBar.vertical.anchors.right = root.right
        flickableItem.ScrollBar.vertical.anchors.top = root.top
        flickableItem.ScrollBar.vertical.anchors.bottom = root.bottom
        //FIXME Results in a "Cannot anchor to an item that is not a parent or a sibling" warning, but we need it to scroll the textfield
        flickableItem.TextArea.flickable = contentItem
        contentItem.anchors.fill = flickableItem;
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
        var sampleItem = flickableItem.itemAt ? flickableItem.itemAt(0, flickableItem.contentY) : null;
        //TODO: config of how many lines the wheel scrolls
        var wheelScrollLines = 3
        var step = Math.min((sampleItem ? sampleItem.height : (Kube.Units.gridUnit + Kube.Units.smallSpacing * 2)) * wheelScrollLines, Kube.Units.gridUnit * 8);
        var y = wheel.pixelDelta.y != 0 ? wheel.pixelDelta.y : (wheel.angleDelta.y > 0 ? step : -step)
        //Ignore 0 events (happens at least with Christians trackpad)
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
