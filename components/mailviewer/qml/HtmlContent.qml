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
import QtWebEngine 1.4

import org.kube.framework 1.0 as Kube

Item {
    id: root
    property string content
    //We have to give it a minimum size so the html content starts to expand
    property int contentHeight: 10;
    property string searchString
    onSearchStringChanged: {
        htmlView.findText(searchString)
    }

    WebEngineView {
        id: htmlView
        anchors.fill: parent
        Component.onCompleted: loadHtml(content, "file:///")
        onContentsSizeChanged: {
            //Resizing pixel by pixel causes some mails to grow indefinitely
            if (contentsSize.height >= root.contentHeight + 5) {
                root.contentHeight = contentsSize.height
            }
        }
        onLoadingChanged: {
            if (loadRequest.status == WebEngineLoadRequest.LoadFailedStatus) {
                console.warn("Failed to load html content.")
                console.warn("Error is ", loadRequest.errorString)
            }
        }
        onLinkHovered: {
            console.debug("Link hovered ", hoveredUrl)
        }
        onNavigationRequested: {
            console.debug("Nav request ", request)
        }
        onNewViewRequested: {
            console.debug("New view request ", request, request.requestedUrl)
            //We ignore requests for new views and open a browser instead
            Qt.openUrlExternally(request.requestedUrl)
        }
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
            //The webview should not steal focus
            focusOnNavigationEnabled: false
        }
        profile: Kube.WebEngineProfile
        onContextMenuRequested: function(request) {
            request.accepted = true
        }
    }
    onContentChanged: {
        htmlView.loadHtml(content, "file:///");
    }
}
