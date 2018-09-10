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

FocusScope {
    id: root
    property var controller

    width: Kube.Units.gridUnit * 7 * 7 + Kube.Units.gridUnit * 2
    height: Kube.Units.gridUnit * 27

    Rectangle {
        anchors {
            fill: parent
        }

        color: Kube.Colors.viewBackgroundColor

        Column {

            anchors {
                fill: parent
                margins: Kube.Units.smallSpacing
            }

            spacing: Kube.Units.largeSpacing

            Kube.TextField {
                width: parent.width
                placeholderText: "Title"
                text: controller.summary
            }
            RowLayout {
                spacing: Kube.Units.smallSpacing

                DayChooser { }

                TimeChooser {
                    visible: !controller.allDay
                }

                Kube.Label {
                    text: " " + qsTr("until") + " "
                }

                DayChooser { }

                TimeChooser {
                   visible: !controller.allDay
                }
            }

            RowLayout {
                spacing: Kube.Units.largeSpacing

                RowLayout {
                    Layout.fillHeight: true
                    Kube.CheckBox {
                        checked: controller.allDay
                        onClicked: {
                            controller.allDay = !controller.allDay
                        }
                    }

                    Kube.Label {
                        text: "All day"
                    }
                }

                Kube.ComboBox {
                    model: ["once", "daily", "weekly"]
                }
            }

            Kube.TextEditor {
                width: parent.width
                height: 200
                text: controller.description
            }
        }
    }
}
