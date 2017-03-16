import QtQuick 2.0
import QtQuick.Controls 2.0
import org.kde.kirigami 1.0 as Kirigami

MouseArea {
    id: popup
    anchors.top: parent.top
    anchors.horizontalCenter: parent.horizontalCenter
    width: Math.max(300, message.contentWidth + (Kirigami.Units.gridUnit * 2))
    height: message.contentHeight + (Kirigami.Units.gridUnit * 2)
    property alias title: message.text
    property alias timeout: hideTimer.interval
    property alias background: bg.color
    visible: opacity > 0
    opacity: 0.0

    Behavior on opacity {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
            property: "opacity"
        }
    }

    Rectangle {
        id: bg
        anchors.fill: parent
    }

    Timer {
        id: hideTimer
        triggeredOnStart: false
        repeat: false
        interval: 5000
        onTriggered: popup.hide()
    }

    function hide() {
        if (hideTimer.running)
            hideTimer.stop()
        popup.opacity = 0.0
    }

    function show() {
        console.warn("Trying to show the notification", title);
        popup.opacity = 1.0
        hideTimer.restart()
    }

    function notify(text) {
        popup.title = text
        bg.color = Kirigami.Theme.highlightColor
        show()
    }

    Label {
        id: message
        anchors.verticalCenter: popup.verticalCenter
        font.pixelSize: 16
        color: Kirigami.Theme.highlightedTextColor
        anchors.left: parent.left
        anchors.leftMargin: Kirigami.Units.gridUnit
        anchors.right: parent.right
        anchors.rightMargin: Kirigami.Units.gridUnit
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        wrapMode: Text.Wrap
    }

    onClicked: hide()
}
