import QtQuick 2.7

ListModel {
    ListElement {
        events: [
        ListElement {
            color: "#af1a6a"
            starts: 1
            duration: 4
            text: "Meeting"
            indention: 0
        },
        ListElement  {
            color: "#134bab"
            starts: 9
            duration: 5
            text: "Sport"
            indention: 0
        }
        ]
    }
    ListElement {
        events: [
        ListElement  {
            color: "#134bab"
            starts: 9
            duration: 5
            text: "Sport"
            indention: 0
        }
        ]
    }
    ListElement {
        events: []
    }
    ListElement {
        events: [
        ListElement {
            color: "#af1a6a"
            starts: 1
            duration: 4
            indention: 0
            text: "Meeting"
        }
        ]
    }
    ListElement {
        events: [
        ListElement {
            color: "#134bab"
            starts: 3
            duration: 5
            indention: 0
            text: "Meeting"
        },
        ListElement {
            color: "#af1a6a"
            starts: 4
            duration: 7
            indention: 1
            text: "Meeting2"
        }
        ]
    }
    ListElement {
        events: []
    }
    ListElement {
        events: []
    }
}
