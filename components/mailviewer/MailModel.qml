import QtQuick 2.4

ListModel {

    ListElement {
        type: "encrypted"
        trusted: false
        trustlevel: "trusted" // "trusted" = green, "unknown" = grey, "dangerous" = red, "wierd" = yellow
        content: [
        ListElement {
            type: "plaintext"
            textContent: "Moin, \n find the forwarded mail below. \n \n - M"
            embeded: false
        },
        ListElement {
            type: "embeded"
            sender: "Senderson"
            date: "05/05/2055"
            content: [
            ListElement{
                type: "plaintext"
                textContent: "sender mc senderson is a sender. sender mc senderson is a sender. sender mc senderson is a  mc senderson is a sender sender mc senderson is a sender sender mc senderson is a sender sender mc senderson is a sender sender mc senderson is a sender sender mc   a sender sender mc  is a sender sender mc senderson is a sendersender mc senderson is a sender"
                embeded: true
            }]
        }
        ]
    }
    ListElement {
        type: "plaintext"
        textContent: "footer mc footerson"
        embeded: false
    }
}
