import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

import org.kube.framework.domain 1.0 as KubeFramework

Item {
    id: root
    property variant message;
    property string html;
    property int desiredHeight: topPartLoader.height;

    Rectangle {
        id: rootRectangle
        anchors.fill: parent
        anchors.margins: 0
        ScrollView {
            id: scrollView
            anchors.fill: parent
            anchors.margins: 0
            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            MessagePartTree {
                id: topPartLoader
                width: rootRectangle.width
                height: topPartLoader.desiredHeight
            }
        }
    }

    KubeFramework.MessageParser {
        id: messageParser
        message: root.message
    }
    html: "XXX"+messageParser.html
}
