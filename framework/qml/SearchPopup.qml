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

FocusScope {
    id: root

    property rect searchArea
    property string backgroundColor: Kube.Colors.darkCharcoalGrey
    property real backgroundOpacity: 0
    property real searchAreaOpacity: backgroundOpacity / 4
    property bool movedSearchBox: false
    property real borderWidth: 3
    property alias textField: textField

    NumberAnimation on backgroundOpacity {
        id: fadeIn
        from: 0
        to: 0.8
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
        root.filter = ""
        fadeOut.start()
    }

    property string filter: ""

    parent: ApplicationWindow.overlay
    anchors.fill: parent
    enabled: false

    property string internalFilter: ""
    Timer {
        id: publishTimer
        interval: 200
        onTriggered: root.filter = root.internalFilter
    }

    function updateFilter(text) {
        if (text.length >= 2) {
            root.internalFilter = text
            publishTimer.restart()
        } else {
            root.internalFilter = text
            publishTimer.restart()
        }
    }

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
            width: root.borderWidth
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
        height: textField.height
        width: Kube.Units.gridUnit * 30

        color: Kube.Colors.darkBackgroundColor

        states: [
            State {
                name: "searchInProgress"
                when: textField.text.length != 0
                PropertyChanges {target: filterField; restoreEntryValues: false; y: root.borderWidth}
                PropertyChanges {target: root; restoreEntryValues: false; searchAreaOpacity: 0}
            }
        ]

        transitions: Transition {
            NumberAnimation { properties: "y"; easing.type: Easing.InOutQuad }
        }

        function clearSearch() {
            textField.text = ""
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

            width: parent.width
            spacing: 0

            Kube.IconButton {
                iconName: Kube.Icons.remove
                activeFocusOnTab: visible
                onClicked: filterField.clearSearch()
            }

            Kube.TextField {
                id: textField
                Layout.fillWidth: true
                placeholderText: qsTr("Filter...")
                onTextChanged: updateFilter(text)
                activeFocusOnTab: visible
                focus: visible
                Keys.onEscapePressed: filterField.clearSearch()
            }
        }
    }
}
