import QtQuick 2.4

Rectangle {
    id: app

    width: 1200
    height: 700

    Rectangle {
        anchors.fill: parent

        color: "black"

        opacity: 0.8

    }

    Rectangle {

        anchors.centerIn: parent

        height: mainColumn.height + 50
        width: parent.width * 0.9

        Column {
            id: mainColumn

            anchors.centerIn: parent

            width: parent.width - 50

            spacing: 10

            Repeater {
                model: MailModel {}

                delegate: Column {
                    id: delegateRoot

                    width: mainColumn.width

                    Loader {
                        id: loader
                    }

                    Component.onCompleted: {
                        switch (model.type) {
                            case "red":
                                loader.source = "Rect2.qml";
                                break;
                            case "green":
                                loader.source = "Rect1.qml";
                                break;
                            case "encrypted":
                                loader.source = "EncryptedPart.qml";
                                break;
                            case "frame":
                                loader.source = "Frame.qml";
                                break;
                            case "plaintext":
                                loader.source = "TextPart.qml";
                                break;
                        }
                    }
                }
            }
        }
    }
}
