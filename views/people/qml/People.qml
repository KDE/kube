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
import QtQuick.Controls 2
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

            Kube.PositiveButton {
                anchors {
                  verticalCenter: parent.verticalCenter
                  left: parent.left
                  leftMargin: Kube.Units.smallSpacing
                }
                text: qsTr("New Contact")
                //visible: stack.depth == 1
                visible: false

                onClicked: {
                    stack.push(personComposer)
                }
            }

            Kube.IconButton {
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: Kube.Units.smallSpacing
                }
                visible: stack.depth > 1
                iconName: Kube.Icons.goBack
                onClicked: {
                    if(stack.depth == 1)
                        root.currentContact = ""
                    stack.pop()
                }
            }
            Kube.TextField {
                id: searchBar
                focus: true
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.5
                placeholderText: qsTr("Search...")
            }
        }

        StackView {
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

                    Keys.onReturnPressed: {
                        GridView.view.currentIndex = index
                        GridView.view.selectObject(model.domainObject)
                    }

                    Rectangle {
                        anchors.fill: parent

                        border.width: 1
                        border.color: Kube.Colors.buttonColor

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
                    Kube.AbstractButton {

                        anchors.fill: parent

                        color: "#00000000"

                        onClicked: {
                            parent.GridView.view.currentIndex = index
                            parent.GridView.view.selectObject(model.domainObject)
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

            PersonPage {
            }

            Kube.Button {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    margins: Kube.Units.largeSpacing
                }
                text: qsTr("Remove")

                onClicked: {
                    contactController.remove()
                    stack.pop()
                }
            }

            Kube.Button {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                text: qsTr("Edit")

                onClicked: {
                    stack.push(personComposer)
                }
            }
        }
    }

    Component {
        id: personComposer

        Rectangle {
            id: personComposerRoot

            color: Kube.Colors.viewBackgroundColor

            PersonComposer {
                contactController: Kube.ContactController {
                    id: contactController
                    contact: root.currentContact
                }
            }

            Kube.PositiveButton {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }

                text: "Save"

                onClicked: {
                    contactController.saveAction.execute()
                    stack.pop()
                }
            }
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

