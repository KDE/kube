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
        },
        State {
            name: "new"
            PropertyChanges { target: eventDisplay; visible: false }
            PropertyChanges { target: eventEditor; visible: true }
        }
        ]

        state: "display"

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


            Kube.TextField {
                id: titleEdit
                anchors {
                    left: parent.left
                    right: parent.right
                }
                placeholderText: "Event Title"
            }

            ColumnLayout {
                anchors {
                    top: titleEdit.bottom
                    topMargin: Kube.Units.largeSpacing
                    left: parent.left
                    right: parent.right
                    bottom: buttons.top
                    bottomMargin: Kube.Units.largeSpacing
                }

                spacing: Kube.Units.smallSpacing

                Kube.Label {
                    Layout.fillWidth: true
                    text: "15:00 bis 17:30"
                }

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

            RowLayout {
                id: buttons

                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }

                spacing: Kube.Units.smallSpacing

                Kube.Button {
                    text: "Discard Changes"
                    onClicked: {
                        root.state =  "display"
                        popup.close()
                    }
                }

                Kube.PositiveButton {
                    text: "Save Changes"
                    onClicked: {
                        root.state = "display"
                        popup.close()
                    }
                }
            }
        }
    }
}
