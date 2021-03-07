/*
  Copyright (C) 2021 Christian Mollekopf, <christian@mkpf.ch>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.9
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.ListDelegate {
    id: delegateRoot
    property bool buttonsVisible: delegateRoot.hovered

    property var mainText
    property var subText
    property var disabled
    property var strikeout
    property var bold
    property var important
    property var dateText
    property var dotColor

    property list<Item> buttons

    signal dropped(var dropAction, var dropTarget)

    height: Kube.Units.gridUnit * 3 + 2 * Kube.Units.smallSpacing

    color: Kube.Colors.viewBackgroundColor
    border.color: Kube.Colors.backgroundColor
    border.width: 1

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        drag.target: parent
        drag.filterChildren: true
        onReleased: {
            var dropAction = parent.Drag.drop()
            delegateRoot.dropped(dropAction, parent.Drag.target)
        }
        onClicked: delegateRoot.clicked()
    }

    states: [
        State {
            name: "dnd"
            when: mouseArea.drag.active

            PropertyChanges {target: mouseArea; cursorShape: Qt.ClosedHandCursor}
            PropertyChanges {target: delegateRoot; opacity: 0.5}
            ParentChange {target: delegateRoot; parent: root; x: x; y: y}
        }
    ]

    Drag.active: mouseArea.drag.active
    Drag.hotSpot.x: mouseArea.mouseX
    Drag.hotSpot.y: mouseArea.mouseY
    Drag.source: delegateRoot

    Item {
        id: content

        anchors {
            fill: parent
            margins: Kube.Units.smallSpacing
        }

        Rectangle {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
            width: Kube.Units.smallSpacing
            height: width
            radius: width / 2

            color: delegateRoot.dotColor
        }
        Column {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin:  Kube.Units.largeSpacing
            }

            Kube.Label{
                id: mainLabel
                width: content.width - Kube.Units.gridUnit * 3
                text: delegateRoot.mainText
                color: delegateRoot.disabled ? delegateRoot.disabledTextColor : delegateRoot.textColor
                font.strikeout: delegateRoot.strikeout
                font.bold: delegateRoot.bold
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
            }
            Kube.Label {
                id: subLabel
                visible: delegateRoot.hovered
                text: delegateRoot.subText
                color: delegateRoot.disabledTextColor
                font.italic: true
                width: delegateRoot.width - Kube.Units.gridUnit * 3
                elide: Text.ElideRight
            }
        }

        Kube.Label {
            id: dateLabel
            anchors {
                right: parent.right
                bottom: parent.bottom
            }

            visible: delegateRoot.date && !delegateRoot.buttonsVisible
            text: delegateRoot.dateText
            font.italic: true
            color: delegateRoot.disabledTextColor
            font.pointSize: Kube.Units.tinyFontSize
        }
    }

    Kube.Icon {
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: Kube.Units.smallSpacing
        }

        visible:  delegateRoot.important && !delegateRoot.buttonsVisible
        iconName: Kube.Icons.isImportant
    }

    Column {
        id: buttons

        anchors {
            right: parent.right
            margins: Kube.Units.smallSpacing
            verticalCenter: parent.verticalCenter
        }

        visible: delegateRoot.buttonsVisible
        opacity: 0.7
        children: delegateRoot.buttons
    }
}
