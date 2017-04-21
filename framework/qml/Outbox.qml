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
import org.kube.framework 1.0 as Kube


Controls.ToolButton {
    id: root

    visible: outboxModel.count > 0

    Kube.OutboxController {
        id: outboxController
    }
    Kube.OutboxModel {
        id: outboxModel
    }
    iconName: Kube.Icons.outbox

    states: [
        State {
            name: "busy"; when: outboxModel.status == Kube.OutboxModel.InProgressStatus
            PropertyChanges { target: root; iconName: Kube.Icons.busy_inverted }
        },
        State {
            name: "error"; when: outboxModel.status == Kube.OutboxModel.ErrorStatus
            PropertyChanges { target: root; iconName: Kube.Icons.error_inverted }
        }
    ]

    Kube.Label {
        id: text
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        width: Kube.Units.smallSpacing
        text: outboxModel.count
        color: Kube.Colors.disabledTextColor
        font.pointSize: 9
    }

    onClicked: dialog.open()

    Kube.Popup {
        id: dialog

        height: content.height + Kirigami.Units.smallSpacing * 2
        width: content.width + Kirigami.Units.smallSpacing * 2

        y: - dialog.height + root.height
        x: root.width

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

                    color: Kube.Colors.viewBackgroundColor
                    border.color: Kube.Colors.backgroundColor
                    border.width: 1

                    Kube.Label {
                        id: subjectLabel
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kirigami.Units.largeSpacing
                        }
                        text: model.subject

                        color: Kube.Colors.textColor
                        opacity: 1
                        states: [
                            State {
                                name: "inprogress"; when: model.status == Kube.OutboxModel.InProgressStatus
                                PropertyChanges { target: subjectLabel; text: "Sending: " + model.subject }
                            },
                            State {
                                name: "error"; when: model.status == Kube.OutboxModel.ErrorStatus
                                PropertyChanges { target: subjectLabel; color: Kube.Colors.warningColor }
                            }
                        ]
                    }

                    Row {
                        anchors {
                            right: parent.right
                            rightMargin: Kube.Units.smallSpacing
                            verticalCenter: parent.verticalCenter
                        }

                        spacing: Kube.Units.smallSpacing

                        Controls.ToolButton {
                            iconName: Kube.Icons.moveToTrash
                            text: qsTr("Delete Mail")
                            tooltip: text
                            onClicked: {
                                outboxController.mail = model.domainObject
                                outboxController.moveToTrashAction.execute()
                            }
                        }

                        Controls.ToolButton {
                            iconName: Kube.Icons.edit
                            text: qsTr("Edit")
                            tooltip: text
                            onClicked: {
                                outboxController.mail = model.domainObject
                                outboxController.editAction.execute()
                            }
                        }
                    }
                }

                clip: true
            }

            Kube.Button {
                id: sendNowButton

                anchors {
                    top: listView.bottom
                    topMargin: Kirigami.Units.smallSpacing
                    horizontalCenter: parent.horizontalCenter
                }

                visible: listView.count != 0

                text: qsTr("Send now")
                enabled: outboxController.sendOutboxAction.enabled
                onClicked: {
                    outboxController.sendOutboxAction.execute()
                }
            }

            Kube.Label {
                anchors.centerIn: parent

                visible: listView.count == 0

                text: qsTr("No pending messages")
            }
        }
    }
}
