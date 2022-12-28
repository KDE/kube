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

import QtQuick 2.15
import QtQuick.Controls 2
import org.kube.framework 1.0 as Kube

ListView {
    id: root
    property Item mouseProxy: scrollHelper
    property int availableWidth: scrollBar.visible ? width - scrollBar.width: width
    implicitHeight: contentHeight

    clip: true
    ScrollBar.vertical: Kube.ScrollBar { id: scrollBar }
    highlightMoveDuration: 100

    add: Transition {
        // FIXME Even though we handle interrupted transitions in the displaced handler,
        // we still occasionally end up with invisible items (e.g. when starting into the inbound view)
        // NumberAnimation { property: "opacity"; from: 0.1; to: 1.0; duration: 100 }
    }

    displaced: Transition {
        //FIXME This causes some delegates to be invisible every now and then (no idea why)
        // NumberAnimation { properties: "x,y"; duration: 50 }
        //Handle interrupted add transitions
        NumberAnimation { property: "opacity"; to: 1.0; }
    }

    Keys.onPressed: {
        if (event.matches(StandardKey.MoveToNextLine)) {
            incrementCurrentIndex()
        } else if (event.matches(StandardKey.MoveToPreviousLine)) {
            decrementCurrentIndex()
        }
    }

    Kube.ScrollHelper {
        id: scrollHelper
        flickable: root
        anchors.fill: root
    }

    onAtYEndChanged: {
        //The fetchMore logic doesn't work in the inbound view (not sure why, it should).
        //However, this does pretty much the same and works.
        if (atYEnd && model && typeof model.tryFetchMore === "function") {
            model.tryFetchMore()
        }
    }
}

