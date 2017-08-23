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
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube


FocusScope {
    id: root

    property var currentContact

    Item {
        id: peopleRoot

        anchors.fill: parent

        Item {
            id: toolbar
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }

            height: searchBar.height + Kube.Units.smallSpacing
            width: parent.width

            Kube.IconButton {
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: Kube.Units.smallSpacing
                }
                visible: stack.depth > 1
                iconName: Kube.Icons.goBack
                onClicked: stack.pop()
            }
            Kube.TextField {
                id: searchBar
                focus: true
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.5
                placeholderText: "Search..."
            }
            Kube.IconButton {
                anchors {
                    top: parent.top
                    right: parent.right
                    rightMargin: Kube.Units.smallSpacing
                }
                iconName: Kube.Icons.addNew
            }
        }

        Controls.StackView {
            id: stack

            anchors {
                top: toolbar.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            initialItem: peoplePage

            clip: true
        }
    }

    Component {
        id: peoplePage

        Rectangle {
            id: peoplePageRoot
            color: Kube.Colors.viewBackgroundColor

            Kube.GridView {
                id: gridView
                anchors {
                    fill: parent
                    margins: Kube.Units.largeSpacing
                }

                activeFocusOnTab: true

                model: Kube.PeopleModel {
                    filter: searchBar.text
                }

                cellWidth: Kube.Units.gridUnit * 10
                cellHeight: Kube.Units.gridUnit * 3

                onActiveFocusChanged: {
                    if (currentIndex < 0) {
                        currentIndex = 0
                    }
                }

                function selectObject(domainObject) {
                    root.currentContact = domainObject
                    stack.push(personPage)
                }

                delegate: Item {
                    id: delegateRoot

                    height: gridView.cellHeight - Kube.Units.smallSpacing * 2
                    width: gridView.cellWidth - Kube.Units.smallSpacing * 2

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            parent.GridView.view.currentIndex = index
                            parent.GridView.view.selectObject(model.domainObject)
                        }
                    }
                    Keys.onReturnPressed: {
                        GridView.view.currentIndex = index
                        GridView.view.selectObject(model.domainObject)
                    }

                    Rectangle {
                        anchors.fill: parent

                        border.width: parent.GridView.view.currentIndex == index ? 2 : 1
                        border.color: parent.GridView.view.currentIndex == index ? Kube.Colors.highlightColor : Kube.Colors.buttonColor

                        Rectangle {
                            id: avatarPlaceholder
                            color: Kube.Colors.buttonColor
                            anchors {
                                top: parent.top
                                left: parent.left
                                bottom: parent.bottom
                            }
                            clip: true

                            width: height
                            Kube.KubeImage {
                                anchors.fill: parent
                                visible: model.imageData != ""
                                imageData: model.imageData
                            }
                            Kube.Icon {
                                anchors.fill: parent
                                visible: model.imageData == ""
                                iconName: Kube.Icons.user
                            }
                        }

                        Column {
                            width: parent.width
                            anchors {
                                left: avatarPlaceholder.right
                                margins: Kube.Units.smallSpacing
                                verticalCenter: parent.verticalCenter
                            }

                            Kube.Label {
                                width: delegateRoot.width - avatarPlaceholder.width - Kube.Units.smallSpacing * 2

                                text: model.firstName
                                elide: Text.ElideRight
                            }

                            Kube.Label {
                                width: delegateRoot.width - avatarPlaceholder.width - Kube.Units.smallSpacing * 2

                                text: model.lastName
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: personPage

        Rectangle {
            id: personPageRoot

            Kube.ContactController {
                id: contactController
                contact: root.currentContact
            }

            color: Kube.Colors.viewBackgroundColor

            Flickable {
                id: personPageFlickable

                anchors {
                    fill: parent
                    leftMargin: Kube.Units.largeSpacing
                }

                Controls.ScrollBar.vertical: Controls.ScrollBar { }
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

                            Row {
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

                    Item {
                        width: parent.width
                        height: Kube.Units.largeSpacing
                    }
                }
            }
        }
    }
}
