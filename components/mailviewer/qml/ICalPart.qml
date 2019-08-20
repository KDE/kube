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
    ColumnLayout {
        visible: controller.state == Kube.InvitationController.Accepted
        Kube.Heading {
            Layout.fillWidth: true
            text: qsTr("You have accepted: \"%1\"").arg(controller.summary)
        }

        Kube.SelectableLabel {
            visible: controller.allDay
            text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM") + (/*DateUtils.sameDay(controller.start, controller.end)*/ true ? "" : " - " + controller.end.toLocaleString(Qt.locale(), "dd. MMMM"))
        }

        Kube.SelectableLabel {
            visible: !controller.allDay
            text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM hh:mm") + " - " + (/*DateUtils.sameDay(controller.start, controller.end)*/ true ? controller.end.toLocaleString(Qt.locale(), "hh:mm") : controller.end.toLocaleString(Qt.locale(), "dd. MMMM hh:mm"))
        }
    }

    ColumnLayout {
        visible: controller.state == Kube.InvitationController.Declined
        Kube.Heading {
            Layout.fillWidth: true
            text: qsTr("You have declined: \"%1\"").arg(controller.summary)
        }

        Kube.SelectableLabel {
            visible: controller.allDay
            text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM") + (/*DateUtils.sameDay(controller.start, controller.end)*/ true ? "" : " - " + controller.end.toLocaleString(Qt.locale(), "dd. MMMM"))
        }

        Kube.SelectableLabel {
            visible: !controller.allDay
            text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM hh:mm") + " - " + (/*DateUtils.sameDay(controller.start, controller.end)*/ true ? controller.end.toLocaleString(Qt.locale(), "hh:mm") : controller.end.toLocaleString(Qt.locale(), "dd. MMMM hh:mm"))
        }
    }

    ColumnLayout {
        visible: controller.state == Kube.InvitationController.Unknown

        Kube.Heading {
            Layout.fillWidth: true
            text: qsTr("You've been invited to: \"%1\"").arg(controller.summary)
        }

        Kube.SelectableLabel {
            visible: controller.allDay
            text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM") + (/*DateUtils.sameDay(controller.start, controller.end)*/ true ? "" : " - " + controller.end.toLocaleString(Qt.locale(), "dd. MMMM"))
        }

        Kube.SelectableLabel {
            visible: !controller.allDay
            text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM hh:mm") + " - " + (/*DateUtils.sameDay(controller.start, controller.end)*/ true ? controller.end.toLocaleString(Qt.locale(), "hh:mm") : controller.end.toLocaleString(Qt.locale(), "dd. MMMM hh:mm"))
        }

        RowLayout {
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

            Kube.CalendarComboBox {
                id: calendarSelector
                Layout.fillWidth: true
                accountId: Kube.Context.currentAccountId
                contentType: "event"
                onSelected: {
                    controller.calendar = calendar
                }
            }

        }

    }

}
