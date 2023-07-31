import QtQuick 2.0
import QtQuick.Controls 2.0

import org.kube.framework 1.0 as Kube


MouseArea {
    id: popup

    property alias title: message.text
    property alias timeout: hideTimer.interval
    property alias background: bg.color

    parent: ApplicationWindow.overlay
    z: 1.0

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
        bg.color = Kube.Colors.buttonColor
        show()
    }

    Timer {
        id: hideTimer
        triggeredOnStart: false
        repeat: false
        interval: 5000
        onTriggered: popup.hide()
    }

    width: Math.max(300, message.contentWidth + (Kube.Units.largeSpacing * 2))
    height: Math.max(50, message.contentHeight + (Kube.Units.largeSpacing * 2))

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
        border.width: 1
        border.color: Kube.Colors.textColor
        opacity: 0.9
    }

    Kube.Label {
        id: message

        anchors {
            verticalCenter: popup.verticalCenter
            left: parent.left
            leftMargin: Kube.Units.largeSpacing
            right: parent.right
            rightMargin: Kube.Units.largeSpacing
        }

        font.pixelSize: 16

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        wrapMode: Text.Wrap
    }

    onClicked: hide()
}
