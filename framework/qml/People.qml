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
import org.kube.components.theme 1.0 as KubeTheme
import org.kube.framework.domain 1.0 as KubeFramework



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

                iconName: KubeTheme.Icons.goBack

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
                    rightMargin: KubeTheme.Units.smallSpacing
                    verticalCenter: parent.verticalCenter
                }

                iconName: KubeTheme.Icons.addNew
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
            color: KubeTheme.Colors.viewBackgroundColor

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
                            topMargin: KubeTheme.Units.largeSpacing
                            left: parent.left
                            leftMargin: KubeTheme.Units.largeSpacing
                        }

                        spacing: KubeTheme.Units.largeSpacing
                        width: peoplePageRoot.width - KubeTheme.Units.largeSpacing * 2

                        Repeater {

                            model: KubeFramework.PeopleModel {
                                filter: searchBar.text
                            }

                            delegate: Rectangle {
                                id: delegateRoot

                                height: KubeTheme.Units.gridUnit * 3
                                width: KubeTheme.Units.gridUnit * 10

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
                                        margins: KubeTheme.Units.smallSpacing
                                        verticalCenter: parent.verticalCenter
                                    }

                                    Text {
                                        width: delegateRoot.width - avatarPlaceholder.width - KubeTheme.Units.smallSpacing * 2

                                        text: model.firstName
                                        elide: Text.ElideRight
                                        color: KubeTheme.Colors.textColor
                                    }

                                    Text {
                                        width: delegateRoot.width - avatarPlaceholder.width - KubeTheme.Units.smallSpacing * 2

                                        text: model.lastName
                                        elide: Text.ElideRight
                                        color: KubeTheme.Colors.textColor
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

            KubeFramework.ContactController {
                id: contactController
                contact: popup.currentContact
            }

            color: KubeTheme.Colors.viewBackgroundColor

            Item {

                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: KubeTheme.Units.largeSpacing
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

                    spacing: KubeTheme.Units.largeSpacing

                    Item {
                        width: parent.width
                        height: KubeTheme.Units.smallSpacing
                    }

                    Item {

                        height: KubeTheme.Units.gridUnit * 8
                        width: personPageRoot.width - KubeTheme.Units.largeSpacing

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
                                leftMargin: KubeTheme.Units.largeSpacing
                            }

                            text: contactController.name //"Michael Tester"
                        }

                        Text {
                            id: jobTitle

                            anchors {
                                top: nameLabel.bottom
                                left: avatar.right
                                leftMargin: KubeTheme.Units.largeSpacing
                            }

                            text: "CIO"
                        }

                        Rectangle {
                            id: company

                            anchors {
                                bottom: avatar.bottom
                                left: avatar.right
                                leftMargin: KubeTheme.Units.largeSpacing
                            }

                            height: KubeTheme.Units.gridUnit * 3
                            width: KubeTheme.Units.gridUnit * 10

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
                                    leftMargin: KubeTheme.Units.smallSpacing
                                }

                                text: "Sauerkraut AG"

                                color: KubeTheme.Colors.textColor
                            }
                        }
                    }

                    Flow {
                        id: emails

                        width: personPageRoot.width - KubeTheme.Units.largeSpacing

                        Repeater {

                            model: contactController.emails

                            Row {
                                spacing: KubeTheme.Units.smallSpacing
                                Text { text: "(main)" }
                                Text { text: modelData ; color: KubeTheme.Colors.highlightColor }
                                Item { width: KubeTheme.Units.smallSpacing; height: 1 }
                            }
                        }

                        Row {
                            spacing: KubeTheme.Units.smallSpacing
                            Text { text: "(alias)"}
                            Text { text: "test.testerson@gmail.com"; color: KubeTheme.Colors.highlightColor }
                            Item { width: KubeTheme.Units.smallSpacing; height: 1 }
                        }

                        Row {
                            spacing: KubeTheme.Units.smallSpacing
                            Text { text: "(private)"}
                            Text { text: "test@gmail.com"; color: KubeTheme.Colors.highlightColor }
                            Item { width: KubeTheme.Units.smallSpacing; height: 1 }
                        }
                    }

                    Flow {
                        id: phone

                        width: personPageRoot.width - KubeTheme.Units.largeSpacing
                        spacing: KubeTheme.Units.smallSpacing

                        Row {
                            spacing: KubeTheme.Units.smallSpacing
                            Text { text: "(inhouse)"}
                            Text { text: "+49812324932"; opacity: 0.6 }
                            Item { width: KubeTheme.Units.smallSpacing; height: 1 }
                        }
                        Row {
                            spacing: KubeTheme.Units.smallSpacing
                            Text { text: "(mobile)"}
                            Text { text: "+49812324932"; opacity: 0.6 }
                            Item { width: KubeTheme.Units.smallSpacing; height: 1 }
                        }
                        Row {
                            spacing: KubeTheme.Units.smallSpacing
                            Text { text: "(private)"}
                            Text { text: "+49812324932"; opacity: 0.6 }
                            Item { width: KubeTheme.Units.smallSpacing; height: 1 }
                        }
                    }

                    Column {
                        id: address

                        width: personPageRoot.width - KubeTheme.Units.largeSpacing

                        Text { text: "Albertstrasse 35a"}
                        Text { text: "81767 Teststadt"}
                        Text { text: "GERMANY" }
                    }

//                     Column {
//
//                         width: parent.width
//
//                         spacing: KubeTheme.Units.smallSpacing
//
//                         Text {
//
//                             text: root.firstname +  " is part of these groups:"
//                         }
//
//                         GroupGrid {
//                             id: groups
//
//                             width: root.width - KubeTheme.Units.largeSpacing
//
//                             model: GroupModel1 {}
//                         }
//                     }

//                     Column {
//
//                         width: parent.width
//
//                         spacing: KubeTheme.Units.smallSpacing
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
//                             width: root.width - KubeTheme.Units.largeSpacing
//
//                             model: PeopleModel2 {}
//                         }
//                     }

                        Item {
                            width: parent.width
                            height: KubeTheme.Units.largeSpacing
                        }
                    }
                }
            }
        }
    }
}
