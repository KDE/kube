import QtQuick 2.4

Item {

    height: mailPart.height
    width: mailPart.width


    Rectangle {
        id: border
        color: "lightgrey"
        height: mailPart.height
        width: 5
    }

    Text {
        id: sender

        anchors {
            left: border.right
            leftMargin: 15
        }

        text: "sent by " + model.sender + " on  " + model.date
        color: "grey"
    }

    MailPart {
        id: mailPart

        anchors.top: sender.bottom

    }

}

