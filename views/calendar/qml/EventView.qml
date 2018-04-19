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
import Qt.labs.calendar 1.0

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root

    property bool daylong

    Rectangle {
        anchors {
            top: parent.top
            topMargin: Kube.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }

        width: Kube.Units.gridUnit * 7 * 7 + Kube.Units.gridUnit * 2
        height: Kube.Units.gridUnit * 27

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
            }
            RowLayout {
                spacing: Kube.Units.smallSpacing

                DayChooser { }

                TimeChooser {
                    visible: !root.daylong
                }

                Kube.Label {
                    text: " " + qsTr("till") + " "
                }

                DayChooser { }

                TimeChooser {
                   visible: !root.daylong

                }
            }

            RowLayout {
                spacing: Kube.Units.largeSpacing

                RowLayout {
                    Layout.fillHeight: true
                    Kube.CheckBox {
                        checked: root.daylong

                        onClicked: {
                            root.daylong = !root.daylong
                        }
                    }

                    Kube.Label {
                        text: "daylong"
                    }
                }

                Kube.ComboBox {
                    model: ["once", "dayly", "weekly"]

                }
            }

            Kube.TextEditor {
                width: parent.width
                height: 200
            }
        }
    }
}
