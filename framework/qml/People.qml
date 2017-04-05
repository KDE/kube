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
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.1

import org.kde.kirigami 1.0 as Kirigami
import org.kube.framework 1.0 as Kube


Popup {
    id: popup

    property var currentContact

    modal: true

    Item {
        id: peopleRoot

        anchors.fill: parent

        ToolBar {
            id: toolbar

            width: parent.width

            Controls.ToolButton {

                anchors.verticalCenter: parent.verticalCenter

                iconName: Kube.Icons.goBack

                onClicked: stack.pop()

                visible: stack. depth > 1
            }

            TextField {
                id: searchBar
                anchors.centerIn: parent

                placeholderText: "Search..."

                width: parent.width * 0.5
            }

            Controls.ToolButton {

                anchors {
                    right: parent.right
                    rightMargin: Kube.Units.smallSpacing
                    verticalCenter: parent.verticalCenter
                }

                iconName: Kube.Icons.addNew
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

            Flickable {

                anchors.fill: parent

                ScrollBar.vertical: ScrollBar { }
                contentHeight: content.height
                clip: true

                Item {
                    id: content

                    height: childrenRect.height

                    Flow {

                        anchors {
                            top: parent.top
                            topMargin: Kube.Units.largeSpacing
                            left: parent.left
                            leftMargin: Kube.Units.largeSpacing
                        }

                        spacing: Kube.Units.largeSpacing
                        width: peoplePageRoot.width - Kube.Units.largeSpacing * 2

                        Repeater {

                            model: Kube.PeopleModel {
                                filter: searchBar.text
                            }

                            delegate: Rectangle {
                                id: delegateRoot

                                height: Kube.Units.gridUnit * 3
                                width: Kube.Units.gridUnit * 10

                                border.width: 1
                                border.color: "lightgrey"

                                MouseArea {
                                    anchors.fill: parent

                                    onClicked: {
                                        popup.currentContact = model.domainObject
                                        stack.push(personPage)
                                    }
                                }

                                Rectangle {
                                    id: avatarPlaceholder

                                    height: parent.height
                                    width: height

                                    color: "lightgrey"
                                }

                                Column {

                                    width: parent.width

                                    anchors {
                                        left: avatarPlaceholder.right
                                        margins: Kube.Units.smallSpacing
                                        verticalCenter: parent.verticalCenter
                                    }

                                    Text {
                                        width: delegateRoot.width - avatarPlaceholder.width - Kube.Units.smallSpacing * 2

                                        text: model.firstName
                                        elide: Text.ElideRight
                                        color: Kube.Colors.textColor
                                    }

                                    Text {
                                        width: delegateRoot.width - avatarPlaceholder.width - Kube.Units.smallSpacing * 2

                                        text: model.lastName
                                        elide: Text.ElideRight
                                        color: Kube.Colors.textColor
                                    }
                                }
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
                contact: popup.currentContact
            }

            color: Kube.Colors.viewBackgroundColor

            Item {

                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: Kube.Units.largeSpacing
                }

                width: parent.width
                height: parent.height


            Flickable {

                anchors.fill: parent

                ScrollBar.vertical: ScrollBar { }
                contentHeight: contentColumn.height

                clip: true

                ColumnLayout {
                    id: contentColumn

                    width: personPageRoot.width

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

                            color: "lightgrey"
                        }

                        Kirigami.Heading {
                            id: nameLabel

                            anchors {
                                top: avatar.top
                                left: avatar.right
                                leftMargin: Kube.Units.largeSpacing
                            }

                            text: contactController.name //"Michael Tester"
                        }

                        Text {
                            id: jobTitle

                            anchors {
                                top: nameLabel.bottom
                                left: avatar.right
                                leftMargin: Kube.Units.largeSpacing
                            }

                            text: "CIO"
                        }

                        Rectangle {
                            id: company

                            anchors {
                                bottom: avatar.bottom
                                left: avatar.right
                                leftMargin: Kube.Units.largeSpacing
                            }

                            height: Kube.Units.gridUnit * 3
                            width: Kube.Units.gridUnit * 10

                            border.width: 1
                            border.color: "lightgrey"

                            Rectangle {
                                id: av

                                height: parent.height
                                width: height

                                color: "lightgrey"
                            }

                            Text {
                                anchors {
                                    verticalCenter: av.verticalCenter
                                    left: av.right
                                    leftMargin: Kube.Units.smallSpacing
                                }

                                text: "Sauerkraut AG"

                                color: Kube.Colors.textColor
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
                                Text { text: "(main)" }
                                Text { text: modelData ; color: Kube.Colors.highlightColor }
                                Item { width: Kube.Units.smallSpacing; height: 1 }
                            }
                        }

                        Row {
                            spacing: Kube.Units.smallSpacing
                            Text { text: "(alias)"}
                            Text { text: "test.testerson@gmail.com"; color: Kube.Colors.highlightColor }
                            Item { width: Kube.Units.smallSpacing; height: 1 }
                        }

                        Row {
                            spacing: Kube.Units.smallSpacing
                            Text { text: "(private)"}
                            Text { text: "test@gmail.com"; color: Kube.Colors.highlightColor }
                            Item { width: Kube.Units.smallSpacing; height: 1 }
                        }
                    }

                    Flow {
                        id: phone

                        width: personPageRoot.width - Kube.Units.largeSpacing
                        spacing: Kube.Units.smallSpacing

                        Row {
                            spacing: Kube.Units.smallSpacing
                            Text { text: "(inhouse)"}
                            Text { text: "+49812324932"; opacity: 0.6 }
                            Item { width: Kube.Units.smallSpacing; height: 1 }
                        }
                        Row {
                            spacing: Kube.Units.smallSpacing
                            Text { text: "(mobile)"}
                            Text { text: "+49812324932"; opacity: 0.6 }
                            Item { width: Kube.Units.smallSpacing; height: 1 }
                        }
                        Row {
                            spacing: Kube.Units.smallSpacing
                            Text { text: "(private)"}
                            Text { text: "+49812324932"; opacity: 0.6 }
                            Item { width: Kube.Units.smallSpacing; height: 1 }
                        }
                    }

                    Column {
                        id: address

                        width: personPageRoot.width - Kube.Units.largeSpacing

                        Text { text: "Albertstrasse 35a"}
                        Text { text: "81767 Teststadt"}
                        Text { text: "GERMANY" }
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
}
