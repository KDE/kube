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
import QtQuick.Controls 2.2
import QtQuick.Window 2

import org.kube.framework 1.0 as Kube

Item {
    id: root
    objectName: "htmlPart"
    property string content
    //We have to give it a minimum size so the html content starts to expand
    property int minimumSize: 10
    property int contentHeight: minimumSize
    property int contentWidth: minimumSize
    property string searchString
    property bool autoLoadImages: false

    onSearchStringChanged: {
        htmlView.findText(searchString)
    }

    Flickable {
        id: flickable
        anchors.fill: parent


        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.horizontal: Kube.ScrollBar {  }

        WebEngineView {
            id: htmlView
            objectName: "htmlView"
            anchors.fill: parent

            Component.onCompleted: loadHtml(content, "file:///")
            onLoadingChanged: {
                if (loadRequest.status == WebEngineView.LoadFailedStatus) {
                    console.warn("Failed to load html content.")
                    console.warn("Error is ", loadRequest.errorString)
                }
                root.contentWidth = Math.max(contentsSize.width, flickable.minimumSize)

                if (loadRequest.status == WebEngineView.LoadSucceededStatus) {
                    runJavaScript("[document.body.scrollHeight, document.body.scrollWidth, document.documentElement.scrollHeight]", function(result) {
                        root.contentHeight = Math.min(Math.max(result[0], result[2]), 4000);
                        root.contentWidth = Math.min(Math.max(result[1], flickable.width), 2000)
                    });
                }
            }
            onLinkHovered: {
                console.debug("Link hovered ", hoveredUrl)
            }
            onNavigationRequested: {
                console.debug("Nav request ", request.navigationType, request.url)
                if (request.navigationType == WebEngineNavigationRequest.LinkClickedNavigation) {
                    Qt.openUrlExternally(request.url)
                    request.action = WebEngineNavigationRequest.IgnoreRequest
                }
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
                javascriptEnabled: true
                javascriptCanOpenWindows: false
                javascriptCanAccessClipboard: false
                hyperlinkAuditingEnabled: false
                fullScreenSupportEnabled: false
                errorPageEnabled: false
                //defaultTextEncoding: ???
                autoLoadImages: root.autoLoadImages
                autoLoadIconsForPage: false
                accelerated2dCanvasEnabled: false
                //The webview should not steal focus
                focusOnNavigationEnabled: false
            }
            profile {
                offTheRecord: true
                httpCacheType: WebEngineProfile.NoCache
                persistentCookiesPolicy: WebEngineProfile.NoPersistentCookies
            }
            onContextMenuRequested: function(request) {
                request.accepted = true
            }
        }
    }
    onContentChanged: {
        htmlView.loadHtml(content, "file:///");
    }
}
