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

import QtQuick 2.4
import QtQuick.Controls 1.3
//TODO import QtWebEngine 1.4
import QtWebEngine 1.3

Item {
    id: root
    property string content: model.content
    //We have to give it a minimum size so the html content starts to expand
    property int contentHeight: 10;

    height: contentHeight
    width: partColumn.width

    WebEngineView {
        id: htmlView
        anchors.fill: parent
        Component.onCompleted: loadHtml(content, "file:///")
        onContentsSizeChanged: {
            root.contentHeight = contentsSize.height;
        }
        onLoadingChanged: {
            if (loadRequest.status === WebEngineView.LoadFailedStatus) {
                console.warn("Failed to load html content.");
                console.warn("Error is ", loadRequest.errorString);
            }
        }
        //TODO The webview should not steal focus (depends on webengine 1.4)
        //focusOnNavigationEnabled: false
        settings {
            webGLEnabled: false
            touchIconsEnabled: false
            spatialNavigationEnabled: false
            screenCaptureEnabled: false
            pluginsEnabled: false
            localStorageEnabled: false
            localContentCanAccessRemoteUrls: false
            localContentCanAccessFileUrls: false
            linksIncludedInFocusChain: false
            javascriptEnabled: false
            javascriptCanOpenWindows: false
            javascriptCanAccessClipboard: false
            hyperlinkAuditingEnabled: false
            fullScreenSupportEnabled: false
            errorPageEnabled: false
            //defaultTextEncoding: ???
            autoLoadImages: true
            autoLoadIconsForPage: false
            accelerated2dCanvasEnabled: false
        }
        //TODO Disable the context menu (depends on webengine 1.4)
        // onContextMenuRequested: function(request) {
        //     request.accepted = true
        // }
    }
    onContentChanged: {
        htmlView.loadHtml(content, "file:///");
    }
}
