/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
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
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Item {
    id: root

    property variant controller
    implicitHeight: listView.height + lineEdit.height
    height: implicitHeight

    Column {
        anchors.fill: parent
        spacing: Kube.Units.smallSpacing

        ListView {
            id: listView
            anchors {
                left: parent.left
                right: parent.right
            }
            height: contentHeight

            model: controller.model

            delegate: Row {
                height: Kube.Units.gridUnit
                spacing: Kube.Units.smallSpacing
                Kube.Label {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: model.isMain
                    text: qsTr("(main)")
                }
                Kube.SelectableLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.email
                }
                Kube.IconButton {
                    anchors.verticalCenter: parent.verticalCenter
                    height: Kube.Units.gridUnit
                    width: height
                    padding: 0
                    iconName: Kube.Icons.listRemove
                    onClicked: root.controller.remove(model.id)
                }
            }
        }

        FocusScope {
            anchors {
                left: parent.left
                right: parent.right
            }
            height: Kube.Units.gridUnit
            focus: true

            Kube.TextButton {
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                }
                id: button
                text: "+ " + qsTr("Add email")
                textColor: Kube.Colors.highlightColor
                focus: true
                onClicked: {
                    lineEdit.visible = true
                    lineEdit.forceActiveFocus()
                }
            }

            Kube.TextField {
                id: lineEdit
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    right: parent.right
                }
                visible: false

                placeholderText: "+ " + qsTr("Add email")
                onAccepted: {
                    root.controller.add({"email": text, "isMain": false});
                    clear()
                    visible = false
                    button.forceActiveFocus(Qt.TabFocusReason)
                }
                // onAborted: {
                //     clear()
                //     visible = false
                //     button.forceActiveFocus(Qt.TabFocusReason)
                // }
            }
        }
    }
}
