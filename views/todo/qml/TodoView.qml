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
// import "dateutils.js" as DateUtils

FocusScope {
    id: root
    property var controller: null

    signal done()

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: eventDetails
        clip: true
        visible: controller
    }

    Component {
        id: eventDetails
        Rectangle {
            color: Kube.Colors.viewBackgroundColor

            ColumnLayout {
                id: contentLayout
                anchors {
                    fill: parent
                    margins: Kube.Units.largeSpacing
                }

                spacing: Kube.Units.smallSpacing

                Kube.Heading {
                    width: parent.width
                    text: controller.summary
                }

                Kube.SelectableLabel {
                    visible: !isNaN(controller.due)
                    text: "Due on: " + controller.due.toLocaleString(Qt.locale(), "dd. MMMM")
                }

                Kube.SelectableLabel {
                    visible: !isNaN(controller.start)
                    text: "Start on: " + controller.start.toLocaleString(Qt.locale(), "dd. MMMM")
                }

                TextEdit {
                    text: controller.description
                    readOnly: true
                    selectByMouse: true
                    color: Kube.Colors.textColor
                    font.family: Kube.Font.fontFamily
                }

                Item {
                    Layout.fillHeight: true
                    width: 1
                }

                RowLayout {
                    Kube.Button {
                        text: qsTr("Remove")
                        onClicked: {
                            root.controller.remove()
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Kube.Button {
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
        TodoEditor {
            controller: root.controller
            editMode: true
            onDone: root.done()
        }
    }
}
