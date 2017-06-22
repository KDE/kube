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

    Flickable {
        id: flick

        contentWidth: root.viewport.width
        contentHeight: Math.max(edit.contentHeight + edit.topPadding + edit.bottomPadding, flick.height)
        clip: true

        function ensureVisible(r)
        {
            if (contentY >= r.y)
                contentY = r.y;
            else if (contentY + height <= r.y + r.height)
                contentY = r.y+r.height-height;
        }

        Kube.TextArea {
        //TODO drop all sizes and use the following once we have qt 5.9
        // Controls2.TextArea.flickable: Kube.TextArea {
            id: edit
            width: flick.contentWidth - edit.leftPadding - edit.rightPadding
            height: flick.contentHeight
            selectByMouse: true
            onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
            wrapMode: TextEdit.Wrap
        }
    }
}
