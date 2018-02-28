 /*
  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.7
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube


Flickable {
    id: personPageFlickable

    anchors {
        fill: parent
        leftMargin: Kube.Units.largeSpacing
    }

    Controls2.ScrollBar.vertical: Kube.ScrollBar { }
    contentHeight: contentColumn.height

    clip: true

    Kube.ScrollHelper {
        flickable: personPageFlickable
        anchors.fill: parent
    }


    ColumnLayout {
        id: contentColumn

        anchors {
            left: parent.left
            right: parent.right
        }

        spacing: Kube.Units.largeSpacing

        Item {
            width: parent.width
            height: Kube.Units.smallSpacing
        }


        Item {

            height: Kube.Units.gridUnit * 8
            width: personPageRoot.width - Kube.Units.largeSpacing

            Rectangle {
                id: avatar

                height: parent.height
                width: height
                Kube.KubeImage {
                    anchors.fill: parent
                    visible: contactController.imageData != ""
                    imageData: contactController.imageData
                }
                Kube.Icon {
                    anchors.fill: parent
                    visible: contactController.imageData == ""
                    iconName: Kube.Icons.user
                }
                color: Kube.Colors.buttonColor
            }

                        Kube.Heading {
                id: nameLabel

                anchors {
                    top: avatar.top
                    left: avatar.right
                    leftMargin: Kube.Units.largeSpacing
                }

                text: contactController.name
            }

            Kube.Label {
                id: jobTitle

                anchors {
                    top: nameLabel.bottom
                    left: avatar.right
                    leftMargin: Kube.Units.largeSpacing
                }

                text: contactController.jobTitle
            }

            Rectangle {
                id: company
                visible: contactController.company != ""

                anchors {
                    bottom: avatar.bottom
                    left: avatar.right
                    leftMargin: Kube.Units.largeSpacing
                }

                height: Kube.Units.gridUnit * 3
                width: Kube.Units.gridUnit * 10

                border.width: 1
                border.color: Kube.Colors.buttonColor

                Rectangle {
                    id: av

                    height: parent.height
                    width: height

                    color: Kube.Colors.buttonColor
                }

                Kube.Label {
                    anchors {
                        verticalCenter: av.verticalCenter
                        left: av.right
                        leftMargin: Kube.Units.smallSpacing
                    }

                    text: contactController.company
                }
            }
        }

        Flow {
            id: emails

            width: personPageRoot.width - Kube.Units.largeSpacing

            Repeater {
                model: contactController.emails

                delegate: Row {
                    spacing: Kube.Units.smallSpacing
                    Kube.Label { text: qsTr("(main)") }
                    Kube.Label { text: modelData ; color: Kube.Colors.highlightColor }
                    Item { width: Kube.Units.smallSpacing; height: 1 }
                }
            }
        }

        Flow {
            id: phone

            width: personPageRoot.width - Kube.Units.largeSpacing
            spacing: Kube.Units.smallSpacing

            Repeater {
                model: contactController.phoneNumbers

                Row {
                    spacing: Kube.Units.smallSpacing
                    Kube.Label { text: qsTr("(main)") }
                    Kube.Label { text: modelData ; opacity: 0.6 }
                    Item { width: Kube.Units.smallSpacing; height: 1 }
                }
            }
        }

        Column {
            id: address

            width: personPageRoot.width - Kube.Units.largeSpacing

            Kube.Label { text: contactController.street }
            Kube.Label { text: contactController.city }
            Kube.Label { text: contactController.country }
        }
        Item {
            width: parent.width
            height: Kube.Units.largeSpacing
        }
    }
}

//                     Column {
//
//                         width: parent.width
//
//                         spacing: Kube.Units.smallSpacing
//
//                         Text {
//
//                             text: root.firstname +  " is part of these groups:"
//                         }
//
//                         GroupGrid {
//                             id: groups
//
//                             width: root.width - Kube.Units.largeSpacing
//
//                             model: GroupModel1 {}
//                         }
//                     }

//                     Column {
//
//                         width: parent.width
//
//                         spacing: Kube.Units.smallSpacing
//
//                         Text {
//                             id: commonPeopleLabel
//
//                             text: root.firstname +  " is associated with:"
//                         }
//
//                         PeopleGrid {
//                             id: commonPeople
//
//                             width: root.width - Kube.Units.largeSpacing
//
//                             model: PeopleModel2 {}
//                         }
//                     }


