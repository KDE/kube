/*
  Copyright (C) 2019 Christian Mollekopf, <mollekopf@kolabsys.com>

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
import QtQuick.Layouts 1

import org.kube.framework 1.0 as Kube

Item {
    id: root

    property string content
    property bool autoLoadImages: false

    property string searchString
    property int contentHeight: childrenRect.height

    Kube.InvitationController {
        id: controller
        Component.onCompleted: loadICal(content)
    }

    states: [
        State {
            name: "exceptioncancelled"
            extend: "cancelled"
            when: controller.method == Kube.InvitationController.Cancel && !isNaN(controller.recurrenceId)
            PropertyChanges {target: label; visible: true; text: qsTr("This is an exception for the event originally occurring at: \"%1\"").arg(controller.recurrenceId.toLocaleString(Qt.locale(), "dd. MMMM hh:mm"))}
        },
        State {
            name: "cancelledaccepted"
            extend: "cancelled"
            when: controller.method == Kube.InvitationController.Cancel && controller.state == Kube.InvitationController.Accepted
            PropertyChanges {target: buttons; visible: false}
        },
        State {
            name: "cancelled"
            when: controller.method == Kube.InvitationController.Cancel
            PropertyChanges {target: heading; text: qsTr("\"%1\" has been cancelled.").arg(controller.summary)}
        },
        State {
            name: "replyAccepted"
            when: controller.method == Kube.InvitationController.Reply && controller.state == Kube.InvitationController.Accepted
            PropertyChanges {target: heading; text: qsTr("%1 has accepted: \"%2\"").arg(controller.name).arg(controller.summary)}
            PropertyChanges {target: buttons; visible: false}
        },
        State {
            name: "replyDeclined"
            when: controller.method == Kube.InvitationController.Reply && controller.state == Kube.InvitationController.Declined
            PropertyChanges {target: heading; text: qsTr("%1 has declined: \"%2\"").arg(controller.name).arg(controller.summary)}
            PropertyChanges {target: buttons; visible: false}
        },
        State {
            name: "replyUnknown"
            when: controller.method == Kube.InvitationController.Reply && controller.state == Kube.InvitationController.Unknown
            PropertyChanges {target: heading; text: qsTr("%1 doesn't yet know about: \"%2\"").arg(controller.name).arg(controller.summary)}
            PropertyChanges {target: buttons; visible: false}
        },
        State {
            name: "updatedexception"
            extend: "updated"
            when: controller.method == Kube.InvitationController.Request && controller.eventState == Kube.InvitationController.Update && !isNaN(controller.recurrenceId)
            PropertyChanges {target: label; visible: true; text: qsTr("This is an exception for the event originally occurring at: \"%1\"").arg(controller.recurrenceId.toLocaleString(Qt.locale(), "dd. MMMM hh:mm"))}
        },
        State {
            name: "updated"
            when: controller.method == Kube.InvitationController.Request && controller.eventState == Kube.InvitationController.Update
            PropertyChanges {target: heading; text: qsTr("The invitation has been updated: \"%1\"").arg(controller.summary)}
        },
        State {
            name: "accepted"
            when: controller.method == Kube.InvitationController.Request && controller.state == Kube.InvitationController.Accepted
            PropertyChanges {target: heading; text: qsTr("You have accepted: \"%1\"").arg(controller.summary)}
            PropertyChanges {target: buttons; visible: false}
        },
        State {
            name: "declined"
            when: controller.method == Kube.InvitationController.Request && controller.state == Kube.InvitationController.Declined
            PropertyChanges {target: heading; text: qsTr("You have declined: \"%1\"").arg(controller.summary)}
            PropertyChanges {target: buttons; visible: false}
        },
        State {
            name: "invited"
            when: controller.method == Kube.InvitationController.Request && controller.state == Kube.InvitationController.Unknown
            PropertyChanges {target: heading; text: qsTr("You've been invited to: \"%1\"").arg(controller.summary)}
        },
        State {
            name: "notinvited"
            when: controller.method == Kube.InvitationController.Request && controller.state == Kube.InvitationController.NoMatch
            PropertyChanges {target: heading; text: qsTr("You are not listed as attendee to: \"%1\"").arg(controller.summary)}
            PropertyChanges {target: buttons; visible: false}
        }
    ]

    ColumnLayout {
        anchors {
            left: parent.left
            right: parent.right
        }
        spacing: Kube.Units.smallSpacing

        Kube.Heading {
            id: heading
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Kube.Label {
            id: label
            visible: false
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Kube.SelectableLabel {
            Layout.fillWidth: true
            visible: controller.allDay
            text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM") + (/*DateUtils.sameDay(controller.start, controller.end)*/ true ? "" : " - " + controller.end.toLocaleString(Qt.locale(), "dd. MMMM"))
            elide: Text.ElideRight
        }

        Kube.SelectableLabel {
            Layout.fillWidth: true
            visible: !controller.allDay
            text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM hh:mm") + " - " + (/*DateUtils.sameDay(controller.start, controller.end)*/ true ? controller.end.toLocaleString(Qt.locale(), "hh:mm") : controller.end.toLocaleString(Qt.locale(), "dd. MMMM hh:mm"))
            elide: Text.ElideRight
        }

        RowLayout {
            id: buttons
            spacing: Kube.Units.smallSpacing
            Layout.fillWidth: true

            Kube.Button {
                text: qsTr("Decline")
                onClicked: {
                    controller.declineAction.execute()
                }
            }

            Kube.PositiveButton {
                text: qsTr("Accept")
                onClicked: {
                    controller.acceptAction.execute()
                }
            }

            Kube.Label {
                text: qsTr("in")
            }

            Kube.EntityComboBox {
                id: calendarSelector
                Layout.fillWidth: true
                accountId: Kube.Context.currentAccountId
                type: "calendar"
                filter: {"contentTypes": "event", "enabled": true}
                onSelected: {
                    if (entity) {
                        controller.calendar = entity
                    }
                }
            }

        }

    }

}
