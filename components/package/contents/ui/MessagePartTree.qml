/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

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

import QtQuick 2.4
import QtQuick.Controls 1.3

Item {
    id: root
    property alias rootIndex: visualModel.rootIndex
    property int nestingLevel: 0
    property int contentHeight: messagePartRect.height
    property int contentWidth: 0
    Rectangle {
        id: messagePartRect
        height: partListView.contentHeight
        width: root.width
        VisualDataModel {
            id: visualModel
            model: messageParser.partTree
            delegate: Rectangle {
                id: delegateRect
                visible: !model.isHidden
                width: childrenRect.width
                height: childrenRect.height
                // color: Qt.rgba(Math.random(),Math.random(),Math.random(),1)
                ContentView {
                    id: contentView
                    anchors.top: delegateRect.top
                    anchors.left: delegateRect.left
                    width: messagePartRect.width
                    height: contentHeight
                    content: model.text
                    isHtml: model.isHtml
                    visible: model.hasContent
                    onVisibleChanged: {
                        //Resize to 0 if it is not visible so the partLoader has the right offset
                        if (!visible) {
                            height = 0
                        }
                    }
                    onContentWidthChanged: {
                        root.contentWidth = contentWidth > root.contentWidth ? contentWidth : root.contentWidth
                    }
                    contentType: model.type
                }
                Loader {
                    id: partLoader
                    anchors.top: contentView.bottom
                    anchors.left: contentView.left
                    visible: model.hasModelChildren
                    active: model.hasModelChildren
                    height: item ? item.contentHeight : 0
                    width: messagePartRect.width
                }
                Component.onCompleted: {
                    if (model.hasModelChildren) {
                        partLoader.source = "MessagePartTree.qml"
                        partLoader.item.rootIndex = visualModel.modelIndex(index)
                        partLoader.item.nestingLevel = root.nestingLevel + 1
                    }
                }
            }
        }

        ListView {
            id: partListView
            model: visualModel
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            height: parent.height
        }
    }
}
