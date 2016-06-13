import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import org.kube.framework.domain 1.0 as KubeFramework

Item {
    id: root
    property variant message;
    property string html;
    property bool enablePartTreeView : false
    property int desiredHeight: enablePartTreeView ? topPartLoader.height+450 : topPartLoader.height;

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
            Column {
                spacing:2
                MessagePartTree {
                    id: topPartLoader
                    width: rootRectangle.width
                    height: topPartLoader.desiredHeight
                }
                TreeView {
                    visible: enablePartTreeView
                    width: 500
                    height: 400
                    TableViewColumn {
                        role: "type"
                        title: "Type"
                        width: 300
                    }
                    TableViewColumn {
                        role: "isHidden"
                        title: "Hidden"
                        width: 60
                    }
                    TableViewColumn {
                        role: "text"
                        title: "Text"
                        width: 600
                    }
                    model: messageParser.partTree
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
