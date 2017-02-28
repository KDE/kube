/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.7
import QtWebEngine 1.3

Item {
    id: root
    property string content;
    property int contentWidth: 0;
    property int contentHeight: 0;
    WebEngineView {
        id: htmlView
        anchors.fill: parent
        onLoadingChanged: {
            // console.debug("Status is ", loadRequest.status);
            // console.debug("Url is ", loadRequest.url);
            if (loadRequest.errorCode) {
                console.warn("Error is ", loadRequest.errorString);
            }
        }
        Component.onCompleted: {
            loadHtml(content, "file:///")
        }
        onContentsSizeChanged: {
            root.contentWidth = contentsSize.width
            root.contentHeight = contentsSize.height
        }
        activeFocusOnPress: false
        settings {
            autoLoadImages: true
            javascriptCanOpenWindows: false
            javascriptEnabled: true
            localStorageEnabled: false
        }
    }
    onContentChanged: {
        htmlView.loadHtml(content, "file:///");
    }
}
