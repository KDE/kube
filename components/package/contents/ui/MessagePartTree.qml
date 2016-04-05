
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtWebKit 3.0

Item {
    id: root
    property alias  rootIndex: visualModel.rootIndex
    property int nestingLevel: 0
    property int desiredHeight: messagePartRect.height
    Rectangle {
        id: messagePartRect
        color: "red"
        height: partListView.contentHeight
        width: root.width
        VisualDataModel {
            id: visualModel
            model: messageParser.partTree
            delegate: Rectangle {
                id: delegateRect
                // visible: !model.isAttachment
                property var countForHeight: true
                width: childrenRect.width
                height: childrenRect.height
                color: Qt.rgba(Math.random(),Math.random(),Math.random(),1)
                ContentView {
                    id: contentView
                    anchors.top: delegateRect.top
                    anchors.left: delegateRect.left
                    width: messagePartRect.width
                    content: model.text
                    isHtml: model.isHtml
                    visible: model.hasContent
                    onVisibleChanged: {
                        //Resize to 0 if it is not visible so the nestedLoaderRect has the right offset
                        if (!visible) {
                            height = 0
                        }
                    }
                    contentType: model.type
                }
                Rectangle {
                    id: nestedLoaderRect
                    visible: model.hasModelChildren
                    height: partLoader.height
                    width: messagePartRect.width
                    anchors.top: contentView.bottom
                    anchors.left: contentView.left
                    anchors.leftMargin: nestingLevel * 5
                    Loader {
                        id: partLoader
                        anchors.top: nestedLoaderRect.top
                        anchors.left: nestedLoaderRect.left
                        width: messagePartRect.width
                        active: model.hasModelChildren
                        height: item ? item.desiredHeight : 0
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
            height: parent.height
        }
    }
}
