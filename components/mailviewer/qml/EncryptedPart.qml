import QtQuick 2.4
import QtQuick.Controls 1.5

Item {

    height: mailPart.height
    width: mailPart.width

   // Rectangle {
     //   id: border

       // border.width: 5
      //  border.color: "lightgreen"
    //}

        BorderImage {

        anchors.fill: parent
        border { left: 40; top: 40; right: 40; bottom: 40 }
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat

        source: model.trusted ?  "bordergreen.png" : "bordergrey.png"
    }

    MailPart {
        id: mailPart

    }

    Rectangle {

        anchors {
            top: parent.top
            right: parent.right
        }

        height: 50
        width: 50

        radius: 100

        color: model.trusted ? "lightgreen" : "lightgrey"

        ToolButton {
            anchors.fill: parent

            iconName: "document-encrypt"
        }
    }
}
