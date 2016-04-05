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
            MessagePartTree {
                id: topPartLoader
                width: scrollView.viewport.width
                height: topPartLoader.desiredHeight
            }
        }
    }

    KubeFramework.MessageParser {
        id: messageParser
        message: root.message
    }
    html: messageParser.html
}
