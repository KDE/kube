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
import org.kube.framework 1.0

T.ComboBox {
    id: root

    implicitWidth: Units.gridUnit * 10
    implicitHeight: Units.gridUnit + Units.smallSpacing * 2

    baselineOffset: contentItem.y + contentItem.baselineOffset

    spacing: Units.largeSpacing
    padding: Units.smallSpacing

    contentItem: Label {
        leftPadding: Units.smallSpacing
        rightPadding: Units.largeSpacing

        text: root.displayText
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Icon {
        x: root.mirrored ? root.leftPadding : root.width - width - root.rightPadding
        y: root.topPadding + (root.availableHeight - height) / 2
        iconName: Icons.goDown
    }

    background: Rectangle {
        border.width: 1
        border.color: root.focus ? Colors.highlightColor : Colors.buttonColor
        color: Colors.viewBackgroundColor
    }

    popup: T.Popup {
        width: root.width
        implicitHeight: Math.min(Units.gridUnit * 5, contentItem.implicitHeight)

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            //FIXME use Kube.Scrollbar once available
            T.ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            color: Colors.backgroundColor
            border.color: Colors.buttonColor
            border.width: 1
        }
    }

    delegate: T.ItemDelegate {
        width: root.popup.width
        height: Units.gridUnit * 1.5

        contentItem: Label {
            padding: Units.smallSpacing
            text: root.textRole ? (Array.isArray(root.model) ? modelData[root.textRole] : model[root.textRole]) : modelData
            color:  root.highlightedIndex === index ? Colors.highlightedTextColor : Colors.textColor
        }

        background: Rectangle {
            color: root.highlightedIndex === index ? Colors.highlightColor : Colors.viewBackgroundColor
            border.width: 1
            border.color: Colors.buttonColor
        }
    }
}
