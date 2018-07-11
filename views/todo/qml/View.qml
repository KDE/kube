/*
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
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

import QtQuick 2.9
import QtQuick.Layouts 1.2

import org.kube.framework 1.0 as Kube


FocusScope {
    id: root

    RowLayout {
        anchors.fill: parent

        spacing: 0

        Rectangle {
            width: Kube.Units.gridUnit * 10
            Layout.fillHeight: parent.height
            color: Kube.Colors.darkBackgroundColor

            Kube.PositiveButton {
                id: newTaskButton
                objectName: "newTaskButton"

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                focus: true
                text: qsTr("New Task")
                onClicked: {}
            }
        }

        Rectangle {

            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Kube.Colors.backgroundColor

            Kube.ListView {
                anchors.fill: parent

                model: 5

                delegate: Kube.ListDelegate {

                    Kube.Label {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kube.Units.largeSpacing
                        }
                        text: "tilte"
                    }

                    Row {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            right: parent.right
                            rightMargin: Kube.Units.largeSpacing
                        }

                        Kube.IconButton {
                            iconName: Kube.Icons.listRemove
                        }

                    }
                }
            }
        }
    }
}






