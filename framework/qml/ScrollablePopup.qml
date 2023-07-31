/*
 *  Copyright (C) 2022 Christian Mollekopf, <christian@mkpf.ch>
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
import QtQuick.Controls 2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.Popup {
    id: root
    modal: true
    parent: ApplicationWindow.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    x: (parent.width - width)/2
    y: Kube.Units.largeSpacing
    width: parent.width / 2
    height: parent.height - Kube.Units.largeSpacing * 2
    clip: true

    default property alias data: flickable.data

    signal accepted

    ColumnLayout {
        anchors.fill: parent

        Flickable {
            id: flickable
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollBar.vertical: Kube.ScrollBar {}
            contentHeight: grid.height
            contentWidth: parent.width

            Keys.onReturnPressed: {
                root.accepted()
            }
        }

        Kube.ScrollHelper {
            id: scrollHelper
            flickable: flickable
            anchors.fill: parent
        }

        Row {
            id: footer
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            height: Kube.Units.gridUnit
            spacing: Kube.Units.smallSpacing

            Kube.Button {
                text: qsTr("Discard")
                onClicked: root.close()
            }

            Kube.Button {
                text: qsTr("Save")
                onClicked: root.accepted()
            }
        }
    }
}
