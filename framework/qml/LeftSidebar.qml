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

Rectangle {
    id: root

    property Component statusDelegate: Kube.StatusBar {
                                           accountId: Kube.Context.currentAccountId
                                       }

    property alias buttons: topLayout.children
    default property alias _contentChildren: mainLayout.children

    width: Kube.Units.gridUnit * 10
    color: Kube.Colors.darkBackgroundColor

    ColumnLayout {
        id: topLayout
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Kube.Units.largeSpacing
        }
        spacing: Kube.Units.largeSpacing
    }

    ColumnLayout {
        id: mainLayout
        anchors {
            top: topLayout.bottom
            topMargin: Kube.Units.largeSpacing
            bottom: statusBarContainer.top
            left: parent.left
            leftMargin: Kube.Units.largeSpacing
            right: parent.right
            rightMargin: Kube.Units.largeSpacing
        }
    }

    Item {
        id: statusBarContainer
        anchors {
            topMargin: Kube.Units.smallSpacing
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: childrenRect.height

        Rectangle {
            id: border
            visible: statusBarLoader.item.visible
            anchors {
                right: parent.right
                left: parent.left
                margins: Kube.Units.smallSpacing
            }
            height: 1
            color: Kube.Colors.viewBackgroundColor
            opacity: 0.3
        }

        Loader {
            id: statusBarLoader
            height: Kube.Units.gridUnit * 2
            anchors {
                top: border.bottom
                left: statusBarContainer.left
                right: statusBarContainer.right
            }
            sourceComponent: root.statusDelegate
        }
    }
}
