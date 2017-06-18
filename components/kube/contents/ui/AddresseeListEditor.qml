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
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Item {
    id: root
    property variant completer
    property alias model: listView.model

    signal added(string text)
    signal removed(string text)

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

            delegate: Rectangle {
                height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 3 // 2 for padding, 1 for spacing to the next item
                width: parent.width
                color: Kube.Colors.buttonColor

                Kube.Label {
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        margins: Kube.Units.smallSpacing
                    }
                    text: display
                    elide: Text.ElideRight
                }

                Rectangle {
                    anchors.bottom: parent.bottom

                    width: parent.width
                    height: Kube.Units.smallSpacing
                    color: Kube.Colors.backgroundColor
                }
            }
        }

        MouseArea {
            height: Kube.Units.gridUnit * Kube.Units.smallSpacing * 2
            width: parent.width
            hoverEnabled: true

            onClicked: {
                lineEdit.visible = true
                lineEdit.forceActiveFocus()
            }

            Kube.Label {
                text: "Add recipient"
                color: Kube.Colors.highlightColor
                font.underline: parent.containsMouse
            }

            Kube.AutocompleteLineEdit {
                id: lineEdit
                anchors {
                    left: parent.left
                    right: parent.right
                }

                visible: false

                placeholderText: "Add recepient"
                model: root.completer.model
                onSearchTermChanged: root.completer.searchString = searchTerm
                onAccepted: {
                    root.added(text);
                    console.warn("Accepted input: ", text)
                    clear()
                    visible = false
                }
            }
        }
    }
}
