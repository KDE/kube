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

import org.kube.framework 1.0 as Kube


Kube.IconButton {
    id: root

    visible: outboxModel.count > 0

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
            rightMargin: 1
            bottomMargin: 1
        }
        width: Kube.Units.smallSpacing
        text: outboxModel.count
        color: Kube.Colors.disabledTextColor
        font.pointSize: Kube.Units.smallFontSize
    }

    onClicked: dialog.open()

    Kube.Popup {
        id: dialog

        height: content.height + Kube.Units.smallSpacing * 2
        width: content.width + Kube.Units.smallSpacing * 2

        y: - dialog.height + root.height
        x: root.width

        Item  {
            id: content

            anchors.centerIn: parent

            width: Kube.Units.gridUnit * 17
            height: listView.count * Kube.Units.gridUnit * 3 + sendNowButton.height + Kube.Units.smallSpacing

            ListView {
                id: listView

                width: parent.width
                height: count * Kube.Units.gridUnit * 3

                model: outboxModel

                delegate: Rectangle {
                    id: delegateRoot

                    height: Kube.Units.gridUnit * 3
                    width: listView.width

                    color: Kube.Colors.viewBackgroundColor
                    border.color: Kube.Colors.backgroundColor
                    border.width: 1

                    Kube.Label {
                        id: subjectLabel
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kube.Units.largeSpacing
                        }
                        text: model.subject

                        color: Kube.Colors.textColor
                        opacity: 1
                        states: [
                            State {
                                name: "inprogress"; when: model.status == Kube.OutboxModel.InProgressStatus
                                PropertyChanges { target: subjectLabel; text: qsTr("Sending: %1").arg(model.subject) }
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

                        Kube.IconButton {
                            iconName: Kube.Icons.moveToTrash
                            onClicked: Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": model.domainObject})
                        }

                        Kube.IconButton {
                            iconName: Kube.Icons.edit
                            onClicked: {
                                Kube.Fabric.postMessage(Kube.Messages.moveToDrafts, {"mail": model.domainObject})
                                //TODO stage upon completion
                                //Kube.Fabric.postMessage(Kube.Messages.edit, {"mail": model.domainObject})
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
                    topMargin: Kube.Units.smallSpacing
                    horizontalCenter: parent.horizontalCenter
                }

                visible: listView.count != 0

                text: qsTr("Send now")
                onClicked: {
                    Kube.Fabric.postMessage(Kube.Messages.sendOutbox, {})
                    dialog.close()
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
