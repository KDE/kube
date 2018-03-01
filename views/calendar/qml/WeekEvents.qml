import QtQuick 2.7

ListModel {
    ListElement {
        events: [
        ListElement {
            color: "#af1a6a"
            starts: 1
            duration: 4
            text: "Meeting"
        },
        ListElement  {
            color: "#134bab"
            starts: 9
            duration: 5
            text: "Sport"
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
            text: "Meeting"
        }
        ]
    }
    ListElement {
        events: [
        ListElement {
            color: "#af1a6a"
            starts: 3
            duration: 5
            text: "Meeting"
        },
        ListElement {
            color: "#af1a6a"
            starts: 9
            duration: 4
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
