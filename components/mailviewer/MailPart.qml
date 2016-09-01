import QtQuick 2.4

Item {
    id: root

    height: partColumn.height + 40
    width: delegateRoot.width

    Column {
        id: partColumn

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 20
        }

        spacing: 10

        Repeater {
            model: content

            delegate: Column {
                id: delegateRoot

                width: partColumn.width

                Loader {
                    id: loader
                }

                Component.onCompleted: {

                    switch (model.type) {
                        case "encrypted":
                            loader.source = "EncryptedPart.qml";
                            break;
                        case "embeded":
                            loader.source = "EmbededPart.qml";
                            break;
                        case "frame":
                            loader.source = "Frame.qml"
                            break;
                        case "plaintext":
                            loader.source = "TextPart.qml";
                            break;
                    }
                }
            }
        }


        Item {
            id: footer

            height: 5
            width: 10
        }
    }
}
