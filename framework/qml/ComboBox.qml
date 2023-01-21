/*
 *  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
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
import QtQuick.Controls 2.0
import QtQuick.Templates 2.0 as T
import org.kube.framework 1.0 as Kube

T.ComboBox {
    id: root

    implicitWidth: Kube.Units.gridUnit * 10
    implicitHeight: Kube.Units.gridUnit + Kube.Units.smallSpacing * 2

    baselineOffset: contentItem.y + contentItem.baselineOffset

    spacing: Kube.Units.largeSpacing
    padding: Kube.Units.smallSpacing

    //Autoselect the first item
    onCountChanged: {
        if (currentIndex < 0) {
            currentIndex = 0
        }
    }

    contentItem: Kube.Label {
        leftPadding: Kube.Units.smallSpacing
        rightPadding: Kube.Units.smallSpacing

        text: root.displayText
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Kube.Icon {
        x: root.mirrored ? root.leftPadding : root.width - width - root.rightPadding
        y: root.topPadding + (root.availableHeight - height) / 2
        iconName: Kube.Icons.goDown
    }

    background: Rectangle {
        border.width: 1
        border.color: root.activeFocus ? Kube.Colors.highlightColor : Kube.Colors.buttonColor
        color: Kube.Colors.viewBackgroundColor
    }

    popup: T.Popup {
        width: root.width
        implicitHeight: Math.min(Kube.Units.gridUnit * 10, contentItem.implicitHeight)

        contentItem: Kube.ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            ScrollBar.vertical: Kube.ScrollBar {}
        }

        background: Rectangle {
            color: Kube.Colors.backgroundColor
            border.color: Kube.Colors.buttonColor
            border.width: 1
        }
    }

    delegate: Kube.ListDelegate {
        width: root.popup.width
        height: Kube.Units.gridUnit * 1.5

        contentItem: Kube.Label {
            padding: Kube.Units.smallSpacing
            text: root.textRole ? (Array.isArray(root.model) ? modelData[root.textRole] : model[root.textRole]) : modelData
            color:  root.highlightedIndex === index ? Kube.Colors.highlightedTextColor : Kube.Colors.textColor
        }

        MouseArea {
            anchors.fill: parent

            onClicked: {
                root.currentIndex = root.highlightedIndex
                root.activated(root.highlightedIndex)
                popup.close()
            }
        }
    }
}
