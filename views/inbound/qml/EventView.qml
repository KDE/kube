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
    property var controller: null

    signal done()

    onControllerChanged: {
        //Wait for a controller to be set before we add a view
        if (controller) {
            stackView.push(eventDetails, StackView.Immediate)
        }
    }

    // function edit() {
    //     var item = stackView.push(editor, StackView.Immediate)
    //     item.forceActiveFocus()
    // }

    StackView {
        id: stackView
        anchors.fill: parent
        clip: true
        visible: controller
    }

    Component {
        id: eventDetails
        Rectangle {
            color: Kube.Colors.paperWhite

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

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Kube.Colors.textColor
                    opacity: 0.5
                }

                Kube.ScrollableTextArea {
                    id: textArea
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    text: Kube.HtmlUtils.toHtml(controller.description)
                    textFormat: Kube.TextArea.RichText
                }

                // RowLayout {
                //     width: parent.width
                //     Kube.Button {
                //         text: qsTr("Remove")
                //         onClicked: {
                //             root.controller.remove()
                //         }
                //     }
                //     Item {
                //         Layout.fillWidth: true
                //     }
                //     Kube.Button {
                //         text: qsTr("Edit")
                //         onClicked: root.edit()
                //     }
                // }
            }
        }
    }

    //Component {
    //    id: editor
    //    TodoEditor {
    //        controller: root.controller
    //        editMode: true
    //        onDone: {
    //            //Reload
    //            root.controller.todo = root.controller.todo
    //            stackView.pop(StackView.Immediate)
    //        }
    //    }
    //}
}
