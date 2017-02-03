 /*
  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>

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

    modal: true

    Controls.SplitView {
        anchors.fill: parent

        Item {
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

                model: 15
                clip: true

                ScrollBar.vertical: ScrollBar {
                    id: scroll
                }

                delegate: Kirigami.AbstractListItem {
                    height: Kirigami.Units.gridUnit * 2.5
                    width: listView.width - scroll.width

                    clip: true

                    Avatar {
                        id: avatar

                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kirigami.Units.smallSpacing
                        }

                        height: parent.height * 0.9
                        width: height

                        name: "Wolfgang Rosenzweig"
                    }

                    Text {
                        id: name

                        anchors {
                            left: avatar.right
                            leftMargin: Kirigami.Units.smallSpacing
                            verticalCenter: avatar.verticalCenter
                        }

                        text: "Wolfgang Rosenzweig"
                        color: Kirigami.Theme.textColor
                    }
                }
            }
        }

        Item {

            KubeFramework.ContactController {
                id: contactController
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

                color: Krigami.Theme.viewBackgroundColor

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
