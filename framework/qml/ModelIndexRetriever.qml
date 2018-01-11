/*
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

import QtQuick 2.4

Repeater {
    id: root
    property var currentData
    property int currentIndex: 0

    function increaseCurrentIndex() {
        if (currentIndex < count - 1) {
            currentIndex = currentIndex + 1
        }
    }

    function decreaseCurrentIndex() {
        if (currentIndex > 0) {
            currentIndex = currentIndex - 1
        }
    }

    onCurrentIndexChanged: {
        currentData = itemAt(currentIndex).currentData
    }
    Item {
        property var currentData: model
        onCurrentDataChanged: {
            if (index == root.currentIndex) {
                root.currentData = model
            }
        }
        visible: false
    }
}
