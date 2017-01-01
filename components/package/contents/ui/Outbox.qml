/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

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

import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as Controls2

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.actions 1.0 as KubeAction
import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.components 1.0 as KubeComponents

ToolButton {
    id: root

    iconName: "mail-folder-outbox"
    tooltip: "outbox"

    onClicked: {
        dialog.visible = dialog.visible ? false : true
    }

    KubeFramework.OutboxController {
        id: outboxController
    }

    //BEGIN Dialog
    Rectangle {
        id: dialog

        property int modelCount: listView.count

        anchors {
            top: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        function calculateHeight() {
            if (modelCount == 0) {
                return Kirigami.Units.gridUnit * 3 + 10
            } else {
                return modelCount * Kirigami.Units.gridUnit * 3 + 10 + sendNowButton.height
            }
        }

        height: calculateHeight()
        width: Kirigami.Units.gridUnit * 20

        color: Kirigami.Theme.backgroundColor
        border.width: 1
        border.color: Kirigami.Theme.highlightColor //TODO change to Kirigami inactive text color once it is available
        radius: 3
        clip: true
        visible: false

        //BEGIN Dialog Content
        Column {
            anchors {
                fill: parent
                margins: 5
            }

            visible: dialog.modelCount != 0

            Controls2.Button {
                id: sendNowButton
                anchors.horizontalCenter: parent.horizontalCenter
                height: Kirigami.Units.gridUnit * 2
                text: qsTr("Send now.")
                onClicked: {
                    outboxController.sendOutboxAction.execute()
                }
            }

            ScrollView {
                id: scrollView

                anchors {
                    left: parent.left
                    right: parent.right
                    margins: 5
                }

                ListView {
                    id: listView

                    model: KubeFramework.OutboxModel {
                    }

                    delegate: Kirigami.AbstractListItem {
                        height: Kirigami.Units.gridUnit * 3

                        Kirigami.Label {
                            anchors.verticalCenter: parent.verticalCenter
                            text: model.subject
                        }
                    }
                }
            }
        }
        Kirigami.Label {
            anchors {
                fill: parent
                margins: 5
                verticalCenter: parent.verticalCenter
            }
            visible: dialog.modelCount == 0
            text: qsTr("No pending messages.")
        }
        //END Dialog Content
    }
    //END Dialog
}
