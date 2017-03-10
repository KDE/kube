 /*
  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
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
import org.kube.framework.domain 1.0 as KubeFramework


Popup {

    id: popup
    modal: true

    property variant currentContact: null

    Controls.SplitView {
        anchors.fill: parent

        Item {
            id: contactList

            height: parent.height
            width: Kirigami.Units.gridUnit * 14

            Item {
                id: toolBar

                width: parent.width - scroll.width
                height: Kirigami.Units.gridUnit * 2

                Rectangle {

                    anchors.centerIn: parent

                    height: Kirigami.Units.gridUnit * 1.5
                    width: parent.width* 0.8

                    color: "#27ae60"
                    clip: true

                    Text {
                        anchors.centerIn: parent

                        clip: true

                        text: "New Contact"

                        color: "white"
                    }
                }
            }

            ListView {
                id: listView

                anchors {
                    top: toolBar.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    topMargin: Kirigami.Units.smallSpacing
                }

                model: KubeFramework.PeopleModel{}
                clip: true

                ScrollBar.vertical: ScrollBar {
                    id: scroll
                }

                onCurrentItemChanged: {
                    popup.currentContact = currentItem.currentData.domainObject;
                }

                delegate: Kirigami.AbstractListItem {
                    height: Kirigami.Units.gridUnit * 2.5
                    width: listView.width - scroll.width

                    property variant currentData: model

                    clip: true

                    states: [
                        State {
                            name: "selected"
                            when: ListView.isCurrentItem
                            PropertyChanges {target: background; color: Kirigami.Theme.highlightColor}
                            PropertyChanges {target: name; color: Kirigami.Theme.highlightedTextColor}
                        },
                        State {
                            name: "hovered"
                            when: mouseArea.containsMouse
                            PropertyChanges {target: background; color: Kirigami.Theme.buttonHoverColor; opacity: 0.7}
                            PropertyChanges {target: name; color: Kirigami.Theme.highlightedTextColor}
                        }
                    ]

                    Avatar {
                        id: avatar

                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kirigami.Units.smallSpacing
                        }

                        height: parent.height * 0.9
                        width: height

                        name: model.name
                    }

                    Text {
                        id: name

                        anchors {
                            left: avatar.right
                            leftMargin: Kirigami.Units.smallSpacing
                            verticalCenter: avatar.verticalCenter
                        }

                        text: model.name
                        color: Kirigami.Theme.textColor
                    }
                }
            }
        }

        Item {
            KubeFramework.ContactController {
                id: contactController
                contact: popup.currentContact
            }

            height: parent.height
            Layout.fillWidth: true

            ToolBar {
                id: detailToolBar

                width: parent.width
                height: Kirigami.Units.gridUnit * 2
            }

            Rectangle {

                anchors {
                    top: detailToolBar.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }

                color: Kirigami.Theme.viewBackgroundColor

                    Row{
                        id: avatar_row

                        height: avatar.height

                        anchors {
                            top: parent.top
                            left: parent.left
                            margins: Kirigami.Units.largeSpacing
                        }

                        spacing: Kirigami.Units.smallSpacing

                        Avatar {
                            id: avatar

                            height: Kirigami.Units.gridUnit * 2.5
                            width: height

                            name: contactController.name
                        }

                        Text {

                            anchors.verticalCenter: parent.verticalCenter

                            color: Kirigami.Theme.textColor
                            opacity: 0.8

                            text: contactController.name

                            font.weight: Font.DemiBold
                        }
                    }

                Text {
                    id: email_label

                        anchors {
                            top: avatar_row.bottom
                            left: avatar_row.left
                            leftMargin:  Kirigami.Units.gridUnit * 2.5
                            topMargin: Kirigami.Units.largeSpacing
                        }

                        color: Kirigami.Theme.textColor
                        text: "Email"
                        font.weight: Font.DemiBold
                        opacity: 0.8
                }

                ColumnLayout {

                    anchors {
                        top: email_label.bottom
                        left: email_label.left
                    }

                    Repeater {
                        model: contactController.emails

                        RowLayout {
                            Text { text: modelData }
                            Controls.ToolButton {
                                iconName: "edit-delete"

                                onClicked: {
                                    contactController.removeEmail(modelData)
                                }
                            }
                        }
                    }

                    RowLayout {
                        TextField {
                            id: newEmail
                        }

                        Button {
                            text: "Add email"

                            onClicked: {
                                contactController.addEmail(newEmail.text)
                                newEmail.text = "";
                            }
                        }
                    }
                }
            }
        }
    }
}
