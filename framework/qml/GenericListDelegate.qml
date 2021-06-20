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
    property var subText: null
    property var disabled: false
    property var active: false
    property var strikeout: false
    property var bold: false
    property var dateText
    property var dotColor: null
    property var counter: 0
    property var subtextVisible: delegateRoot.hovered
    property var subtextDisabled: false
    property Component buttonDelegate: null
    property Component statusDelegate: null


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

       property color unreadColor: delegateRoot.disabled ? delegateRoot.disabledTextColor : ((delegateRoot.active && !delegateRoot.highlighted) ? Kube.Colors.highlightColor : delegateRoot.textColor)

        Rectangle {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
            visible: delegateRoot.dotColor
            width: Kube.Units.smallSpacing
            height: width
            radius: width / 2

            color: delegateRoot.dotColor ? delegateRoot.dotColor : ""
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
                color: content.unreadColor
                font.strikeout: delegateRoot.strikeout
                font.bold: delegateRoot.bold
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
            }
            Kube.Label {
                id: subLabel
                visible: delegateRoot.subtextVisible
                text: delegateRoot.subText ? delegateRoot.subText : ""
                color: delegateRoot.subtextDisabled ? delegateRoot.disabledTextColor : delegateRoot.textColor
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

            visible: delegateRoot.dateText && !delegateRoot.buttonsVisible
            text: delegateRoot.dateText
            font.italic: true
            color: delegateRoot.disabledTextColor
            font.pointSize: Kube.Units.tinyFontSize
        }
       Kube.Label {
           id: counter
           anchors {
               right: parent.right
               margins: Kube.Units.smallSpacing
           }
           text: delegateRoot.counter ? delegateRoot.counter : ""
           color: content.unreadColor
           visible: delegateRoot.counter > 1 && !delegateRoot.buttonsVisible

       }
    }

    Loader {
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            // We use twice the margin to compensate for button padding
            margins: delegateRoot.buttonsVisible ? Kube.Units.smallSpacing : 2 * Kube.Units.smallSpacing
        }
        active: delegateRoot.buttonsVisible && delegateRoot.buttonDelegate || delegateRoot.statusDelegate
        opacity: delegateRoot.buttonsVisible ? 0.7 : 1
        sourceComponent: delegateRoot.buttonsVisible ? delegateRoot.buttonDelegate : delegateRoot.statusDelegate
    }
}
