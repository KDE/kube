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
import QtQuick.Dialogs 1.0 as Dialogs


import org.kube.framework 1.0 as Kube


Flickable {
    id: root

    property var contactController

    anchors {
        fill: parent
        leftMargin: Kube.Units.largeSpacing
    }

    ScrollBar.vertical: Kube.ScrollBar { }
    contentHeight: contentColumn.height

    clip: true

    Kube.ScrollHelper {
        flickable: root
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
            width: root.width - Kube.Units.largeSpacing

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

                Kube.AbstractButton {
                    anchors.fill: parent

                    color: "#00000000"

                    onClicked: {
                        fileDialog.open()
                    }

                    Dialogs.FileDialog {
                        id: fileDialog
                        title: qsTr("Choose an Avatar")
                        folder: shortcuts.home
                        selectFolder: false
                        selectExisting: true
                        selectMultiple: false
                        onAccepted: {
                            //TODO
                        }
                    }
                }
            }

            Row {
                id: nameRow
                anchors {
                    left: avatar.right
                    leftMargin: Kube.Units.largeSpacing
                }

                spacing: Kube.Units.smallSpacing

                Kube.TextField {
                    width: Kube.Units.gridUnit * 15
                    placeholderText: qsTr("First name")
                    backgroundColor: "white"
                    text: contactController.firstName
                    onTextChanged: contactController.firstName = text
                }

                Kube.TextField {
                    width: Kube.Units.gridUnit * 15
                    placeholderText: qsTr("Last name")
                    backgroundColor: "white"
                    text: contactController.lastName
                    onTextChanged: contactController.lastName = text
                }
            }

            Kube.TextField {
                id: jobTitle

                anchors {
                    top: nameRow.bottom
                    left: avatar.right
                    topMargin: Kube.Units.smallSpacing
                    leftMargin: Kube.Units.largeSpacing
                }

                width: Kube.Units.gridUnit * 20
                text: contactController.jobTitle
                onTextChanged: contactController.jobTitle = text
                placeholderText: qsTr("Job Title")
                backgroundColor: "white"
            }

            Kube.TextField {
                id: company

                anchors {
                    bottom: avatar.bottom
                    left: avatar.right
                    leftMargin: Kube.Units.largeSpacing
                }
                width: Kube.Units.gridUnit * 20

                placeholderText: qsTr("Company")
                text: contactController.company
                onTextChanged: contactController.company = text
                backgroundColor: "white"
            }
        }


        Column {
            width: root.width - Kube.Units.largeSpacing
            spacing: Kube.Units.smallSpacing

            Kube.Label {
                text: qsTr("Email")
            }
            MailListEditor {
                controller: contactController.mails
            }
        }

        Column {
            width: root.width - Kube.Units.largeSpacing
            spacing: Kube.Units.smallSpacing

            Kube.Label {
                text: qsTr("Phone")
            }

            PhoneListEditor {
                controller: contactController.phones
            }
        }

        Column {
            id: address

            width: root.width - Kube.Units.largeSpacing
            spacing: Kube.Units.smallSpacing

            Kube.Label {
                text: "Address"
            }

            Kube.TextField {
                width: Kube.Units.gridUnit * 20
                text: contactController.street
                onTextChanged: contactController.stree = text
                placeholderText: qsTr("Street")
                backgroundColor: "white"
            }
            Kube.TextField {
                width: Kube.Units.gridUnit * 20
                text: contactController.city
                onTextChanged: contactController.city = text
                placeholderText: qsTr("City")
                backgroundColor: "white"
            }
            Kube.TextField {
                width: Kube.Units.gridUnit * 20
                text: contactController.country
                onTextChanged: contactController.country = text
                placeholderText: qsTr("Country")
                backgroundColor: "white"
            }
        }

        Column{
            width: root.width - Kube.Units.largeSpacing
            spacing: Kube.Units.smallSpacing

            Kube.Label {
                text: "Addressbook"
            }
            Kube.ComboBox {
                width: Kube.Units.gridUnit * 20

                model: Kube.EntityModel {
                    id: addressbookModel
                    type: "addressbook"
                    roles: ["name"]
                }
                textRole: "name"
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    contactController.addressbook = addressbookModel.data(currentIndex).object
                }
            }
        }

        Item {
            width: parent.width
            height: Kube.Units.largeSpacing
        }
    }
}
