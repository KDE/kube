import QtQuick 2.0
import QtQuick.Controls 2.0
import org.kde.kirigami 1.0 as Kirigami

MouseArea {
    id: popup

    property alias title: message.text
    property alias timeout: hideTimer.interval
    property alias background: bg.color

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
        bg.color = Kirigami.Theme.textColor
        show()
    }

    Timer {
        id: hideTimer
        triggeredOnStart: false
        repeat: false
        interval: 5000
        onTriggered: popup.hide()
    }

    anchors {
        top: parent.top
        horizontalCenter: parent.horizontalCenter
    }

    width: Math.max(300, message.contentWidth + (Kirigami.Units.largeSpacing * 2))
    height: Math.max(50, message.contentHeight + (Kirigami.Units.largeSpacing * 2))

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

        opacity: 0.6
    }

    Label {
        id: message

        anchors {
            verticalCenter: popup.verticalCenter
            left: parent.left
            leftMargin: Kirigami.Units.largeSpacing
            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing
        }

        font.pixelSize: 16

        color: Kirigami.Theme.highlightedTextColor
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        wrapMode: Text.Wrap
    }

    onClicked: hide()
}
