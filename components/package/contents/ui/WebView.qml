import QtQuick 2.4
import QtQuick.Controls 1.3
import QtWebKit 3.0
// import QtWebEngine 1.3 //This would give use contentsSize
// import QtWebEngine 1.2

Item {
    id: root
    property string content;
    // property int contentWidth: 500;
    // property int contentHeight: 500;
    property int contentWidth: htmlView.contentWidth;
    property int contentHeight: htmlView.contentHeight;
    // width: htmlView.width
    // height: htmlView.height
    WebView {
        id: htmlView
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: contentWidth
        height: contentHeight
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
        Component.onCompleted: loadHtml(content, "file:///")
    }
    // WebEngineView {
    //     id: htmlView
    //     anchors.fill: parent
    //     onLoadingChanged: {
    //         console.warn("Error is ", loadRequest.errorString);
    //         console.warn("Status is ", loadRequest.status);
    //     }
    //     Component.onCompleted: loadHtml(content, "file:///")
    // }
    onContentChanged: {
        htmlView.loadHtml(content, "file:///");
    }
}
