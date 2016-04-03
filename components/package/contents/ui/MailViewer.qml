import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtWebKit 3.0

import org.kube.framework.domain 1.0 as KubeFramework

Item {
    id: root
    property variant message;
    property string html;

    Component {
        id: messagePartComponent
        Rectangle {
            id: messagePartRect
            property alias  rootIndex: visualModel.rootIndex
            property int nestingLevel: 0
            color: "red"
            height: partListView.height
            // implicitHeight: partListView.height
            onHeightChanged: {
                console.warn("component size updated: " + width + "*"+ height)
            }
            VisualDataModel {
                id: visualModel
                model: messageParser.partTree
                onRootIndexChanged: {
                    console.warn('Got root index ' + rootIndex)
                    console.warn('Got children ' + model.hasModelChildren)
                }
                delegate: Rectangle {
                    id: delegateRect
                    // visible: !model.isAttachment
                    property var countForHeight: true
                    // width: column.width
                    // height: column.height
                    width: childrenRect.width
                    height: childrenRect.height
                    color: Qt.rgba(Math.random(),Math.random(),Math.random(),1)
                    Rectangle {
                        //FIXME use something that can cleanly switch between plaintext and html view instead of trying to hide the right thing
                        id: contentRect
                        border.width: 2
                        border.color: "black"
                        anchors.top: delegateRect.top
                        anchors.left: delegateRect.left
                        visible: model.hasContent
                        anchors.leftMargin: nestingLevel * 5
                        // color: Qt.rgba(Math.random(),Math.random(),Math.random(),1)
                        color: "green"
                        height: model.hasContent ? childrenRect.height : 0
                        width: messagePartRect.width
                        Text {
                            anchors.top: contentRect.top
                            anchors.left: contentRect.left
                            id: text
                            visible: !model.isHtml
                            //FIXME doesn't work, so childrenRect above is to large in the html case
                            // height: model.isHtml ? 0 : height
                            text: model.text
                        }
                        WebView {
                            id: htmlView
                            //The webview doesn't resize to the content, so set a fixed size
                            anchors.top: contentRect.top
                            anchors.left: contentRect.left
                            width: messagePartRect.width - 100
                            height: model.isHtml ? 600 : 0
                            visible: model.isHtml
                            onNavigationRequested: {
                                // detect URL scheme prefix, most likely an external link
                                var schemaRE = /^\w+:/;
                                if (schemaRE.test(request.url)) {
                                    request.action = WebView.AcceptRequest;
                                } else {
                                    request.action = WebView.IgnoreRequest;
                                    // delegate request.url here
                                }
                            }
                            onLoadingChanged: {
                                console.warn("Error is ", loadRequest.errorString);
                                console.warn("Status is ", loadRequest.status);
                            }
                        }
                    }
                    Rectangle {
                        id: nestedLoaderRect
                        visible: model.hasModelChildren
                        height: partLoader.height
                        // height: 10
                        width: messagePartRect.width
                        // anchors.top: delegateRect.top
                        // anchors.left: delegateRect.left
                        anchors.top: contentRect.bottom
                        anchors.left: contentRect.left
                        anchors.leftMargin: nestingLevel * 5
                        onHeightChanged: {
                            console.warn("Nested loader rect changed: " + height)
                        }
                        // width: partLoader.width
                        color: "yellow"
                        Loader {
                            id: partLoader
                            anchors.top: nestedLoaderRect.top
                            anchors.left: nestedLoaderRect.left
                            width: messagePartRect.width
                        }
                    }
                    Component.onCompleted: {
                        htmlView.loadHtml(model.text, "file:///");
                        // console.warn("Completed loading part " + text);
                        if (model.hasModelChildren) {
                            console.warn("Loading a new subpart");
                            partLoader.sourceComponent = messagePartComponent
                            partLoader.item.rootIndex = visualModel.modelIndex(index)
                            partLoader.item.nestingLevel = messagePartRect.nestingLevel + 1
                        }
                    }
                }
            }

            ListView {
                id: partListView
                model: visualModel
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.right: parent.right
                height: contentHeight
            }
        }
    }

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
                Loader {
                    id: topPartLoader
                    anchors.fill: parent
                    sourceComponent: messagePartComponent
                    onStatusChanged: {
                        console.warn('Loaded')
                        if (topPartLoader.status == Loader.Ready) console.warn('Loaded and ready')
                        console.warn('size' + item.height)
                    }
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
