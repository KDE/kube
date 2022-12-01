/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3

import org.kube.framework 1.0 as Kube
import "dateutils.js" as DateUtils

FocusScope {
    id: root
    property var controller

    implicitWidth: stackView.currentItem.implicitWidth
    implicitHeight: stackView.currentItem.implicitHeight

    signal done()

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: eventDetails
        clip: true
    }

    Component {
        id: eventDetails
        Rectangle {
            implicitWidth: contentLayout.implicitWidth + 2 * Kube.Units.largeSpacing
            implicitHeight: contentLayout.implicitHeight + 2 * Kube.Units.largeSpacing
            color: Kube.Colors.viewBackgroundColor

            ColumnLayout {
                id: contentLayout
                anchors {
                    fill: parent
                    margins: Kube.Units.largeSpacing
                }

                spacing: Kube.Units.smallSpacing

                Kube.Heading {
                    Layout.fillWidth: true
                    text: controller.summary
                    elide: Text.ElideRight
                }

                Kube.SelectableLabel {
                    visible: controller.allDay
                    text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM") + (DateUtils.sameDay(controller.start, controller.end) ? "" : " - " + controller.end.toLocaleString(Qt.locale(), "dd. MMMM"))
                }

                Kube.SelectableLabel {
                    visible: !controller.allDay
                    text: controller.start.toLocaleString(Qt.locale(), "dd. MMMM hh:mm") + " - " + (DateUtils.sameDay(controller.start, controller.end) ? controller.end.toLocaleString(Qt.locale(), "hh:mm") : controller.end.toLocaleString(Qt.locale(), "dd. MMMM hh:mm"))
                }

                Kube.SelectableLabel {
                    visible: controller.recurring
                    text: qsTr("repeats %1").arg(controller.recurrenceString)
                }

                Kube.SelectableLabel {
                    text: "@" + controller.location
                    copyText: controller.location
                    visible: controller.location
                }

                Kube.SelectableLabel {
                    text: qsTr("Organizer: %1").arg(controller.organizer)
                    visible: controller.organizer
                }

                Flow {
                    Layout.fillWidth: true
                    visible: attendeeRepeater.count
                    height: childrenRect.height
                    spacing: Kube.Units.smallSpacing
                    Kube.SelectableLabel {
                        text: qsTr("Attending:")
                        visible: controller.organizer
                    }
                    Repeater {
                        id: attendeeRepeater
                        model: controller.attendees.model
                        delegate: Kube.Label {
                            text: qsTr("%1").arg(model.name) + (index == (attendeeRepeater.count - 1) ? "" : ",")
                            elide: Text.ElideRight
                        }
                    }
                }

                Flickable {
                    id: flickable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.horizontal: Kube.ScrollBar {  }
                    contentHeight: textArea.height
                    contentWidth: textArea.width
                    clip: true
                    Kube.TextArea {
                        id: textArea
                        width: flickable.width
                        text: controller.description
                    }
                    Kube.ScrollHelper {
                        anchors.fill: parent
                        flickable: flickable
                    }
                }

                RowLayout {
                    Kube.Button {
                        text: qsTr("Remove")
                        onClicked: {
                            root.controller.remove()
                            root.done()
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Kube.Button {
                        enabled: controller.ourEvent
                        text: qsTr("Edit")
                        onClicked: {
                            stackView.push(editor, StackView.Immediate)
                        }
                    }

                }
            }
        }
    }

    Component {
        id: editor
        EventEditor {
            controller: root.controller
            editMode: true
            onDone: root.done()
        }
    }
}
