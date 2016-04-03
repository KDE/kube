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
            property alias  rootIndex: visualModel.rootIndex
            color: "red"
            width: childrenRect.width + 20
            height: childrenRect.height + 20
            // height: 1500
            VisualDataModel {
                id: visualModel
                model: messageParser.partTree
                onRootIndexChanged: {
                    console.warn('Got root index ' + rootIndex)
                    console.warn('Got children ' + model.hasModelChildren)
                }
                delegate: Rectangle {
                    // width: 800
                    // height: 800
                    width: childrenRect.width + 20
                    height: childrenRect.height + 20
                    color: Qt.rgba(Math.random(),Math.random(),Math.random(),1)

                    // Text {
                    //     id: topText
                    //     // text: "Showing: " + rootIndex + index
                    //     text: model.text
                    // }
                    Column {
                        // leftPadding: 10
                        spacing: 5

                        Text {
                            // text: "Showing: " + rootIndex + index
                            text: model.text
                        }
                        // WebView {
                        //     width: 800
                        //     height: 800
                        //     // anchors.fill: parent
                        //     id: htmlView
                        //     // anchors.fill: parent
                        //     onNavigationRequested: {
                        //         // detect URL scheme prefix, most likely an external link
                        //         var schemaRE = /^\w+:/;
                        //         if (schemaRE.test(request.url)) {
                        //             request.action = WebView.AcceptRequest;
                        //         } else {
                        //             request.action = WebView.IgnoreRequest;
                        //             // delegate request.url here
                        //         }
                        //     }
                        //     onLoadingChanged: {
                        //         console.warn("Error is ", loadRequest.errorString);
                        //         console.warn("Status is ", loadRequest.status);
                        //     }
                        // }

                        Loader {
                            id: partLoader
                            // anchors.left: topText.left + 10
                            // anchors.topMargin: 10
                            // anchors.top: topText.bottom
                        }
                    }
                    Component.onCompleted: {
                        // htmlView.loadHtml(model.text, "file:///");
                        console.warn("Completed loading part " + text);
                        if (model.hasModelChildren) {
                            console.warn("Loading a new subpart");
                            partLoader.sourceComponent = messagePartComponent
                            partLoader.item.rootIndex = visualModel.modelIndex(index)
                        }
                    }
                }
            }

            ListView {
                // anchors.fill: parent
                model: visualModel
            }
        }
    }

    Rectangle {
        // width: 1000
        // height: 900
        color: blue
        anchors.fill: parent
        ScrollView {
            anchors.fill: parent
            Loader {
                anchors.fill: parent
                sourceComponent: messagePartComponent
            }
        }
    }

    // WebView {
    //     id: webview
    //     anchors.fill: parent
    //     onNavigationRequested: {
    //         // detect URL scheme prefix, most likely an external link
    //         var schemaRE = /^\w+:/;
    //         if (schemaRE.test(request.url)) {
    //             request.action = WebView.AcceptRequest;
    //         } else {
    //             request.action = WebView.IgnoreRequest;
    //             // delegate request.url here
    //         }
    //     }
    //     onLoadingChanged: {
    //         console.warn("Error is ", loadRequest.errorString);
    //         console.warn("Status is ", loadRequest.status);
    //     }
    // }
    //

    onHtmlChanged: {
        // console.warn("HTML is ", html);
        // The file:/// argument is necessary so local icons are found
        // webview.loadHtml(html, "file:///");
        console.warn("MessageChanged ");
        // listView.model = messageParser.partTree
        // partText.text = messageParser.partTree.text
        // messageParser.partTree.
    }

    KubeFramework.MessageParser {
        id: messageParser
        message: root.message
    }
    html: messageParser.html
}
