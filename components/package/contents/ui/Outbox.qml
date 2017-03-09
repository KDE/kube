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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.actions 1.0 as KubeAction
import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.components 1.0 as KubeComponents

Button {
    id: root

    text: "outbox"

    onClicked: {
        dialog.visible = dialog.visible ? false : true
    }

    KubeFramework.OutboxController {
        id: outboxController
    }

    Popup {
        id: dialog

        height: content.height + Kirigami.Units.smallSpacing * 2
        width: content.width + Kirigami.Units.smallSpacing * 2

        y: - dialog.height + root.height
        x: root.width

        modal: true

        Item  {
            id: content

            anchors.centerIn: parent

            width: Kirigami.Units.gridUnit * 17
            height: listView.count * Kirigami.Units.gridUnit * 3 + sendNowButton.height + Kirigami.Units.smallSpacing

            ListView {
                id: listView

                width: parent.width
                height: count * Kirigami.Units.gridUnit * 3

                model: KubeFramework.OutboxModel {}

                delegate: Rectangle {
                    id: delegateRoot

                    height: Kirigami.Units.gridUnit * 3
                    width: listView.width

                    color: Kirigami.Theme.viewBackgroundColor
                    border.color: Kirigami.Theme.backgroundColor
                    border.width: 1

                    Label {
                        id: subjectLabel
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kirigami.Units.largeSpacing
                        }
                        text: model.subject

                        //FIXME use theme color
                        color: model.status == "error" ? "red" : Kirigami.Theme.textColor
                        opacity: model.status == "sent" ? 0.5 : 1
                    }
                }

                clip: true
            }

            Button {
                id: sendNowButton

                anchors {
                    top: listView.bottom
                    topMargin: Kirigami.Units.smallSpacing
                    horizontalCenter: parent.horizontalCenter
                }

                visible: listView.count != 0

                text: qsTr("Send now")
                onClicked: {
                    outboxController.sendOutboxAction.execute()
                }
            }

            Label {
                anchors.centerIn: parent

                visible: listView.count == 0

                text: qsTr("No pending messages")
            }
        }
    }
}
