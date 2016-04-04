
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtWebKit 3.0

Item {
    id: root
    property alias  rootIndex: visualModel.rootIndex
    property int nestingLevel: 0
    height: partListView.height
    Rectangle {
        id: messagePartRect
        color: "red"
        height: partListView.height
        width: root.width
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
                ContentView {
                    id: contentLoader
                    anchors.top: delegateRect.top
                    anchors.left: delegateRect.left
                    width: messagePartRect.width
                    content: model.text
                    isHtml: model.isHtml
                    visible: model.hasContent
                }
                Rectangle {
                    id: nestedLoaderRect
                    visible: model.hasModelChildren
                    height: partLoader.height
                    width: messagePartRect.width
                    anchors.top: contentLoader.bottom
                    anchors.left: contentLoader.left
                    anchors.leftMargin: nestingLevel * 5
                    onHeightChanged: {
                        console.warn("Nested loader rect changed: " + height)
                    }
                    color: "yellow"
                    Loader {
                        id: partLoader
                        anchors.top: nestedLoaderRect.top
                        anchors.left: nestedLoaderRect.left
                        width: messagePartRect.width
                        active: model.hasModelChildren
                    }
                }
                Component.onCompleted: {
                    console.warn("Completed loading part " + text);
                    if (model.hasModelChildren) {
                        console.warn("Loading a new subpart");
                        partLoader.source = "MessagePartTree.qml"
                        partLoader.item.rootIndex = visualModel.modelIndex(index)
                        partLoader.item.nestingLevel = root.nestingLevel + 1
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
