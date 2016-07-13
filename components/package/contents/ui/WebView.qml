import QtQuick 2.4
import QtQuick.Controls 1.3
import QtWebKit 3.0
// import QtWebEngine 1.3 //This would give use contentsSize
import QtWebEngine 1.2

Item {
    id: root
    property string content;
    property int contentWidth: helperView.contentWidth;
    property int contentHeight: helperView.contentHeight;
    //FIXME workaround until QtWebEngine 1.3 with contentsSize
    WebView {
        id: helperView
        visible: false
        Component.onCompleted: loadHtml(content, "file:///")
    }
    WebEngineView {
        id: htmlView
        anchors.fill: parent
        onLoadingChanged: {
            console.warn("Error is ", loadRequest.errorString);
            console.warn("Status is ", loadRequest.status);
        }
        Component.onCompleted: loadHtml(content, "file:///")
    }
    onContentChanged: {
        htmlView.loadHtml(content, "file:///");
        helperView.loadHtml(content, "file:///");
    }
}
