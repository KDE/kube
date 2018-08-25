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

Item {
    id: app

    width: 900
    height: 600

    Kube.Popup {
        id: popup

        x: app.width * 0.05
        y: app.height * 0.15

        width: app.width * 0.7
        height: app.height * 0.7


        Item {
            id: root
            anchors.fill: parent

            states: [
            State {
                name: "display"
                PropertyChanges { target: editButton; visible: true }
                PropertyChanges { target: discardButton; visible: false }
                PropertyChanges { target: saveButton; visible: false }
            },
            State {
                name: "edit"
                PropertyChanges { target: editButton; visible: false }
                PropertyChanges { target: discardButton; visible: true }
                PropertyChanges { target: saveButton; visible: true }
            }
            ]

            state: "display"

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

            RowLayout {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }

                Kube.Button {
                    id: editButton
                    text: "Edit"

                    onClicked: {
                        root.state = "edit"
                    }
                }
                Kube.Button {
                    id: discardButton
                    text:"Discard Changes"

                    onClicked: {
                        root.state = "display"
                    }
                }
                Kube.PositiveButton {
                    id: saveButton
                    text: "Save Changes"

                    onClicked: {
                        root.state = "display"
                    }
                }
            }
        }
        visible: true
    }
}
