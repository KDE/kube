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
import QtQuick.Dialogs 1.0 as Dialogs


import org.kube.framework 1.0 as Kube


Flickable {
    id: personComposerFlickable

    anchors {
        fill: parent
        leftMargin: Kube.Units.largeSpacing
    }

    Controls2.ScrollBar.vertical: Kube.ScrollBar { }
    contentHeight: contentColumn.height

    clip: true

    Kube.ScrollHelper {
        flickable: personComposerFlickable
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
            width: personComposerRoot.width - Kube.Units.largeSpacing

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
                        fileDialogComponent.createObject(parent)
                    }

                    Component {
                        id: fileDialogComponent
                        Dialogs.FileDialog {
                            id: fileDialog
                            visible: true
                            title: "Choose an Avatar"
                            selectFolder: false
                            onAccepted: {
                                //TODO
                            }
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
                    placeholderText: qsTr("First Name")
                }

                Kube.TextField {
                    width: Kube.Units.gridUnit * 15
                    placeholderText: qsTr("Last Name")
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
                placeholderText: qsTr("Job Title")
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
            }
        }


        Column {
            width: personComposerRoot.width - Kube.Units.largeSpacing
            spacing: Kube.Units.smallSpacing

            Kube.Label {
                text: qsTr("Email")
            }
            Flow {
                id: emails

                width: personComposerRoot.width - Kube.Units.largeSpacing

                Repeater {
                    model: contactController.emails

                    delegate: Row {
                        spacing: Kube.Units.smallSpacing
                        Kube.Label { text: qsTr("(main)") }
                        Kube.TextField { text: modelData ; color: Kube.Colors.highlightColor }
                        Item { width: Kube.Units.smallSpacing; height: 1 }
                    }
                }
            }
            Kube.Button {
                text: qsTr("Add")
            }
        }

        Column {
            width: personComposerRoot.width - Kube.Units.largeSpacing
            spacing: Kube.Units.smallSpacing

            Kube.Label {
                text: qsTr("Phone")
            }

            Flow {
                id: phone

                width: personComposerRoot.width - Kube.Units.largeSpacing
                spacing: Kube.Units.smallSpacing

                Repeater {
                    model: contactController.phoneNumbers

                    Row {
                        spacing: Kube.Units.smallSpacing
                        Kube.Label { text: qsTr("(main)") }
                        Kube.TextField { text: modelData ; opacity: 0.6 }
                        Item { width: Kube.Units.smallSpacing; height: 1 }
                    }
                }
            }
            Kube.Button {
                text: qsTr("Add")
            }
        }

        Column{
            id: address

            width: personComposerRoot.width - Kube.Units.largeSpacing
            spacing: Kube.Units.smallSpacing

            Kube.Label {
                text: "Address"
            }

            Kube.TextField {
                width: Kube.Units.gridUnit * 20
                text: contactController.street
                placeholderText: qsTr("Street")
            }
            Kube.TextField {
                width: Kube.Units.gridUnit * 20
                text: contactController.city
                placeholderText: qsTr("City")
            }
            Kube.TextField {
                width: Kube.Units.gridUnit * 20
                text: contactController.country
                placeholderText: qsTr("Country")
            }
        }
        Item {
            width: parent.width
            height: Kube.Units.largeSpacing
        }
    }
}
