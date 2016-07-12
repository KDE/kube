import QtQuick 2.4
import QtQuick.Controls 1.3

Item {
    id: root
    property string content;
    property int contentWidth: 0;
    property int contentHeight: textView.contentHeight;
    Text {
        id: textView
        wrapMode: Text.WordWrap
        anchors.fill: parent
        text: content
    }
}
