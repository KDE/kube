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


import QtQuick 2.9
import QtQuick.Controls 2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Item {
    id: root

    property rect searchArea
    property string backgroundColor: Kube.Colors.darkCharcoalGrey
    property real backgroundOpacity: 0
    property real searchAreaOpacity: backgroundOpacity / 4

    NumberAnimation on backgroundOpacity {
        id: fadeIn
        from: 0
        to: 0.6
        duration: 100
    }

    Component.onCompleted: fadeIn.start()

    NumberAnimation on backgroundOpacity {
        id: fadeOut
        running: false
        to: 0
        duration: 100
        onRunningChanged: {
             if (!running) {
                root.destroy()
             }
        }
    }

    function close() {
        fadeOut.start()
    }

    property string filter: ""

    parent: ApplicationWindow.overlay
    anchors.fill: parent
    enabled: false

    //left
    Rectangle {
        x: 0
        y: 0
        width: searchArea.x
        height: parent.height
        color: parent.backgroundColor
        opacity: parent.backgroundOpacity
    }
    //bottom
    Rectangle {
        x: searchArea.x
        y: searchArea.y + searchArea.height
        width: searchArea.width
        height: parent.height - y
        color: parent.backgroundColor
        opacity: parent.backgroundOpacity
    }
    //right
    Rectangle {
        x: searchArea.x + searchArea.width
        y: 0
        width: parent.width - x
        height: parent.height
        color: parent.backgroundColor
        opacity: parent.backgroundOpacity
    }
    //bottom
    Rectangle {
        x: searchArea.x
        y: 0
        width: searchArea.width
        height: searchArea.y
        color: parent.backgroundColor
        opacity: parent.backgroundOpacity
    }
    //outline
    Rectangle {
        x: searchArea.x
        y: searchArea.y
        width: searchArea.width
        height: searchArea.height
        color: "transparent"
        border {
            width: 3
            color: Kube.Colors.highlightColor
        }
        Rectangle {
            anchors.fill: parent
            color: parent.parent.backgroundColor
            opacity: parent.parent.searchAreaOpacity
        }
    }


    Rectangle {
        id: filterField
        enabled: true
        parent: ApplicationWindow.overlay

        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        y: parent.height / 3
        height: Kube.Units.gridUnit * 2
        width: Kube.Units.gridUnit * 30
        radius: Kube.Units.smallSpacing

        color: Kube.Colors.buttonColor

        states: [
            State {
                name: "searchInProgress"
                when: find.text.length != 0
                PropertyChanges {target: filterField; y: Kube.Units.gridUnit}
                PropertyChanges {target: root; searchAreaOpacity: 0}
            }
        ]

        transitions: Transition {
            NumberAnimation { properties: "y"; easing.type: Easing.InOutQuad }
        }

        function clearSearch() {
            find.text = ""
            root.close()
        }

        Shortcut {
            sequences: [StandardKey.Cancel]
            onActivated: filterField.clearSearch()
        }

        RowLayout {
            anchors {
                verticalCenter: parent.verticalCenter
            }

            width: parent.width - Kube.Units.smallSpacing
            spacing: 0

            Kube.IconButton {
                iconName: Kube.Icons.remove
                activeFocusOnTab: visible
                onClicked: filterField.clearSearch()
            }

            Kube.TextField {
                id: find
                Layout.fillWidth: true
                placeholderText: qsTr("Filter...")
                onTextChanged: root.filter = text
                activeFocusOnTab: visible
                focus: visible
                Keys.onEscapePressed: filterField.clearSearch()
            }
        }
    }
}
