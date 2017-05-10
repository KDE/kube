/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
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


import QtQuick 2.7
import QtQuick.Controls 1.3
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.View {
    id: root
    Rectangle {
        width: Kube.Units.gridUnit * 10
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        color: Kube.Colors.textColor
        focus: true

        Column {
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
            }
            Kube.PositiveButton {
                id: newMailButton
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                text: qsTr("Compose New")
                onClicked: root.incrementCurrentIndex()
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        ColumnLayout {
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
            }
            Kube.TextField {
                id: subject
                Layout.fillWidth: true

                placeholderText: "Enter Subject..."
                // text: composerController.subject
                // onTextChanged: composerController.subject = text;
            }

            Controls2.TextArea {
                id: content
                Layout.fillWidth: true
                Layout.fillHeight: true

                // text: composerController.body
                // onTextChanged: composerController.body = text;
            }
        }
    }

    Rectangle {
        width: Kube.Units.gridUnit * 10
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        anchors {
            top: parent.top
            bottom: parent.bottom
        }
    }
}
