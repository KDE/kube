import QtQuick 2.4
import QtQuick.Controls 1.3

Item {
    id: root
    property alias rootIndex: visualModel.rootIndex
    property int nestingLevel: 0
    property int contentHeight: messagePartRect.height
    property int contentWidth: 0
    Rectangle {
        id: messagePartRect
        height: partListView.contentHeight
        width: root.width
        VisualDataModel {
            id: visualModel
            model: messageParser.partTree
            delegate: Rectangle {
                id: delegateRect
                visible: !model.isHidden
                width: childrenRect.width
                height: childrenRect.height
                // color: Qt.rgba(Math.random(),Math.random(),Math.random(),1)
                ContentView {
                    id: contentView
                    anchors.top: delegateRect.top
                    anchors.left: delegateRect.left
                    width: messagePartRect.width
                    height: contentHeight
                    content: model.text
                    isHtml: model.isHtml
                    visible: model.hasContent
                    onVisibleChanged: {
                        //Resize to 0 if it is not visible so the partLoader has the right offset
                        if (!visible) {
                            height = 0
                        }
                    }
                    onContentWidthChanged: {
                        root.contentWidth = contentWidth > root.contentWidth ? contentWidth : root.contentWidth
                    }
                    contentType: model.type
                }
                Loader {
                    id: partLoader
                    anchors.top: contentView.bottom
                    anchors.left: contentView.left
                    visible: model.hasModelChildren
                    active: model.hasModelChildren
                    height: item ? item.contentHeight : 0
                    width: messagePartRect.width
                }
                Component.onCompleted: {
                    if (model.hasModelChildren) {
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
