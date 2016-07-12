import QtQuick 2.4
import QtQuick.Controls 1.3

Item {
    id: root
    property int nestingLevel;
    property bool isHtml;
    property string content;
    property string contentType;
    property int contentWidth: contentLoader.item.contentWidth
    property int contentHeight: contentLoader.item.contentHeight
    Rectangle {
        id: contentRect

        //Only for development
        // border.width: 1
        // border.color: "black"
        // radius: 5
        // anchors.leftMargin: nestingLevel * 5
        anchors.fill: parent

        Loader {
            id: contentLoader
            property string content: root.content
            anchors.fill: parent
            sourceComponent: isHtml ? htmlComponent : textComponent
        }

        Component {
            id: textComponent
            TextView {
                content: root.content
            }
        }
        Component {
            id: htmlComponent
            WebView {
                content: root.content
            }
        }
    }
}
