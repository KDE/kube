import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtWebKit 3.0

import org.kube.framework.domain 1.0 as KubeFramework

Item {
    id: root
    property variant message;
    property string html;

    Rectangle {
        id: rootRectangle
        anchors.fill: parent
        ScrollView {
            id: scrollView
            anchors.fill: parent
            Rectangle {
                id: toplevelRectangle
                width: scrollView.viewport.width
                height: topPartLoader.item.height
                onHeightChanged: {
                    console.warn("toplevel component size updated: " + width + "*"+ height)
                }
                color: "red"
                MessagePartTree {
                    id: topPartLoader
                    anchors.fill: parent
                    // sourceComponent: messagePartComponent
                    // onStatusChanged: {
                    //     console.warn('Loaded')
                    //     if (topPartLoader.status == Loader.Ready) console.warn('Loaded and ready')
                    //     console.warn('size' + item.height)
                    // }
                }
            }
        }
    }

    KubeFramework.MessageParser {
        id: messageParser
        message: root.message
    }
    html: messageParser.html
}
