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
import QtQuick.Controls 1.3 as Controls

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.actions 1.0 as KubeAction
import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.components 1.0 as KubeComponents
import org.kube.components.theme 1.0 as KubeTheme

Button {
    id: root

    text: outboxModel.count > 0 ? "outbox (" + outboxModel.count + ")" : "outbox"
    contentItem: Item {
        Text {
            text: parent.text
            font: parent.font
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        Controls.ToolButton {
            id: statusIcon
            anchors {
                right: parent.right
            }
            visible: false
            states: [
                State {
                    name: "busy"; when: outboxModel.status == KubeFramework.OutboxModel.InProgressStatus 
                    PropertyChanges { target: statusIcon; iconName: KubeTheme.Icons.busy; visible: true }
                },
                State {
                    name: "error"; when: outboxModel.status == KubeFramework.OutboxModel.ErrorStatus 
                    PropertyChanges { target: statusIcon; iconName: KubeTheme.Icons.error; visible: true }
                }
            ]
        }
    }

    onClicked: {
        dialog.visible = dialog.visible ? false : true
    }

    KubeFramework.OutboxController {
        id: outboxController
    }

    KubeFramework.OutboxModel {
        id: outboxModel
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

                model: outboxModel

                delegate: Rectangle {
                    id: delegateRoot

                    height: Kirigami.Units.gridUnit * 3
                    width: listView.width

                    color: KubeTheme.Colors.viewBackgroundColor
                    border.color: KubeTheme.Colors.backgroundColor
                    border.width: 1

                    Label {
                        id: subjectLabel
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kirigami.Units.largeSpacing
                        }
                        text: model.subject

                        color: KubeTheme.Colors.textColor
                        opacity: 1
                        states: [
                            State {
                                name: "inprogress"; when: model.status == KubeFramework.OutboxModel.InProgressStatus
                                PropertyChanges { target: subjectLabel; text: "Sending: " + model.subject }
                            },
                            State {
                                name: "error"; when: model.status == KubeFramework.OutboxModel.ErrorStatus
                                PropertyChanges { target: subjectLabel; color: KubeTheme.Colors.warningColor }
                            }
                        ]
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
