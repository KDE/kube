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

import org.kube.framework 1.0 as Kube

ScrollView {
    id: root
    property alias text: edit.text

    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

    function ensureVisible(r)
    {
        if (flickableItem.contentY >= r.y)
            flickableItem.contentY = r.y;
        else if (flickableItem.contentY + height <= r.y + r.height)
            flickableItem.contentY = r.y + r.height - height + edit.padding;
    }

    Kube.TextArea {
    //TODO drop all sizes and use the following once we have qt 5.9
    // Controls2.TextArea.flickable: Kube.TextArea {
        id: edit
        width: root.viewport.width
        height: Math.max(edit.contentHeight + edit.topPadding + edit.bottomPadding, root.height)
        selectByMouse: true
        onCursorRectangleChanged: root.ensureVisible(cursorRectangle)
        wrapMode: TextEdit.Wrap
    }
}
