/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.4
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.Popup {
    id: popup

    Item {
        id: root

        states: [
        State {
            name: "display"
            PropertyChanges { target: eventDisplay; visible: true }
            PropertyChanges { target: eventEditor; visible: false }

        },
        State {
            name: "edit"
            PropertyChanges { target: eventDisplay; visible: false }
            PropertyChanges { target: eventEditor; visible: true }
            PropertyChanges { target: deleteButton; visible: true }
            PropertyChanges { target: abortButton; visible: false }
            PropertyChanges { target: saveButton; visible: true }
            PropertyChanges { target: discardButton; visible: true }
            PropertyChanges { target: createButton; visible: false }
        },
        State {
            name: "new"
            PropertyChanges { target: eventDisplay; visible: false }
            PropertyChanges { target: eventEditor; visible: true }
            PropertyChanges { target: deleteButton; visible: false }
            PropertyChanges { target: abortButton; visible: true }
            PropertyChanges { target: saveButton; visible: false }
            PropertyChanges { target: discardButton; visible: false }
            PropertyChanges { target: createButton; visible: true }
        }
        ]

        state: "new"

        anchors.fill: parent

        Item {
            id: eventDisplay

            anchors.fill: parent

            Kube.Heading {
                id: title
                text: "Event Title"
            }

            ColumnLayout {
                anchors {
                    margins: Kube.Units.largeSpacing
                    top: title.bottom
                }
                spacing: Kube.Units.smallSpacing

                Kube.Label {
                    text: "15:00 bis 17:30"
                }

                Kube.Label {
                    text: "Location"
                }

                Kube.Label {
                    text: "Description"
                }
            }

            Kube.Button {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                }

                text: "Delete"
                onClicked: {
                    popup.close()
                }
            }

            Kube.Button {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }

                text: "Edit"
                onClicked: {
                    root.state = "edit"
                }
            }
        }

        Item {
            id: eventEditor

            anchors.fill: parent

            ColumnLayout {

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    bottom: buttons.top
                    bottomMargin: Kube.Units.largeSpacing
                }

                spacing: Kube.Units.largeSpacing


                Kube.TextField {
                    id: titleEdit
                    Layout.fillWidth: true
                    placeholderText: "Event Title"
                }

                ColumnLayout {
                    id: dateAndTimeChooser

                    states: [
                    State {
                        name: "regular"
                        PropertyChanges {target: fromTime; visible: true}
                        PropertyChanges {target: tillTime; visible: true}
                    },
                    State {
                        name: "daylong"
                        PropertyChanges {target: fromTime; visible: false}
                        PropertyChanges {target: tillTime; visible: false}
                    }
                    ]
                    state: regular

                    spacing: Kube.Units.smallSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Kube.Units.largeSpacing
                        RowLayout {
                            spacing: Kube.Units.smallSpacing

                            DayChooser {}
                            TimeChooser {
                                id: fromTime
                            }
                        }
                        Kube.Label {
                            text: "till"
                        }
                        RowLayout {
                            spacing: Kube.Units.smallSpacing

                            DayChooser {}
                            TimeChooser {
                                id: tillTime
                            }
                        }
                    }

                    RowLayout {
                        spacing: Kube.Units.smallSpacing
                        Kube.CheckBox {
                            onClicked: {
                                checked ? dateAndTimeChooser.state = "daylong" :  dateAndTimeChooser.state = "regular"
                            }
                        }
                        Kube.Label {
                            text: "daylong"
                        }
                    }
                }

                ColumnLayout {
                    spacing: Kube.Units.smallSpacing
                    Layout.fillWidth: true
                    Kube.TextField {
                        Layout.fillWidth: true
                        placeholderText: "Location"
                    }

                    Kube.TextEditor {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        //TODO placeholderText: "Description"
                    }
                }
            }

            RowLayout {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                }

                Kube.Button {
                    id: deleteButton
                    text: "Delete"
                    onClicked: {
                    }
                }
                Kube.Button {
                    id: abortButton
                    text: "Abort"
                    onClicked: {
                        popup.close()
                    }
                }
            }

            RowLayout {
                id: buttons

                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }

                spacing: Kube.Units.smallSpacing

                Kube.Button {
                    id: discardButton
                    text: "Discard Changes"
                    onClicked: {
                        root.state =  "display"
                    }
                }

                Kube.PositiveButton {
                    id: saveButton
                    text: "Save Changes"
                    onClicked: {
                        root.state = "display"
                    }
                }

                Kube.PositiveButton {
                    id: createButton
                    text: "Create Event"
                    onClicked: {
                        popup.close()
                    }
                }
            }
        }
    }
}
