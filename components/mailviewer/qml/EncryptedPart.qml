import QtQuick 2.4
import QtQuick.Controls 1.5

Item {

    height: mailPart.height + 20
    width: mailPart.width + 20

        BorderImage {

        anchors.fill: parent
        border { left: 40; top: 40; right: 40; bottom: 40 }
        horizontalTileMode: BorderImage.Round
        verticalTileMode: BorderImage.Round

        source: "securityborders" + model.securityLevel + ".png"
    }

    MailPart {
        id: mailPart

        anchors.centerIn: parent

    }
}
