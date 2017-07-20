/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsystems.com>

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
import QtQuick.Controls 1.4
import QtQuick.Controls 2 as Controls2
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQml.Models 2.2

import org.kube.framework 1.0 as Kube

Flickable {
    id: root

    default property alias __columns: treeView.__columns
    property alias model: treeView.model
    property alias currentIndex: treeView.currentIndex

    signal dropped(QtObject drop, QtObject model)
    signal activated(var index)

    Controls2.ScrollBar.vertical: Controls2.ScrollBar {}
    clip: true
    contentWidth: root.width
    contentHeight: treeView.implicitHeight
    Kube.ScrollHelper {
        id: scrollHelper
        flickable: root
    }

    TreeView {
        id: treeView

        anchors {
            left: parent.left
            right: parent.right
        }
        implicitHeight: __listView.contentItem.height
        height: __listView.contentItem.height

        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        Kube.MouseProxy {
            anchors.fill: parent
            target: scrollHelper
            forwardWheelEvents: true
        }

        flickableItem.boundsBehavior: Flickable.StopAtBounds

        selection: ItemSelectionModel {
            model: treeView.model
            //TODO once we don't loose focus to the next view
            // onCurrentChanged: {
            //     treeView.activated(selection.currentIndex)
            // }
        }

        function selectFirst()
        {
            treeView.selection.setCurrentIndex(model.index(0, 0), ItemSelectionModel.ClearAndSelect)
        }

        function selectNext()
        {
            var nextIndex = model.sibling(selection.currentIndex.row + 1, 0, selection.currentIndex)
            if (nextIndex.valid) {
                treeView.selection.setCurrentIndex(nextIndex, ItemSelectionModel.ClearAndSelect)
            } else {
                var childIndex = model.index(0, 0, selection.currentIndex)
                if (childIndex.valid) {
                    if (!treeView.isExpanded(selection.currentIndex)) {
                        treeView.expand(selection.currentIndex)
                    }
                    treeView.selection.setCurrentIndex(childIndex, ItemSelectionModel.ClearAndSelect)
                }
            }
        }

        function selectPrevious()
        {
            var previousIndex = model.sibling(selection.currentIndex.row - 1, 0, selection.currentIndex)
            if (previousIndex.valid) {
                treeView.selection.setCurrentIndex(previousIndex, ItemSelectionModel.ClearAndSelect)
            } else {
                var parentIndex = model.parent(selection.currentIndex)
                if (parentIndex.valid) {
                    treeView.selection.setCurrentIndex(parentIndex, ItemSelectionModel.ClearAndSelect)
                }
            }
        }

        onActiveFocusChanged: {
            //Set an initially focused item when the list view receives focus
            if (activeFocus && !selection.hasSelection) {
                selectFirst()
            }
        }

        Keys.onDownPressed: {
            if (!selection.hasSelection) {
                selectFirst()
            } else {
                selectNext();
            }
        }

        Keys.onUpPressed: selectPrevious()
        Keys.onReturnPressed: treeView.activated(selection.currentIndex)

        //Forward the signal because on a desktopsystem activated is only triggerd by double clicks
        onClicked: treeView.activated(index)

        onActivated: root.activated(index)

        alternatingRowColors: false
        headerVisible: false

        style: TreeViewStyle {

            rowDelegate: Rectangle {
                color: styleData.selected ? Kube.Colors.highlightColor : Kube.Colors.textColor
                height: Kube.Units.gridUnit * 1.5
                width: parent.width
            }

            frame: Rectangle {
                color: Kube.Colors.textColor
            }

            branchDelegate: Item {
                width: 16
                height: 16

                Kube.Label  {
                    anchors.centerIn: parent

                    color: Kube.Colors.viewBackgroundColor
                    text: styleData.isExpanded ? "-" : "+"
                }

                //radius: styleData.isExpanded ? 0 : 100
            }

            itemDelegate: Item {

                DropArea {
                    anchors.fill: parent

                    Rectangle {
                        anchors.fill: parent
                        color: Kube.Colors.viewBackgroundColor
                        opacity: 0.3
                        visible: parent.containsDrag
                    }
                    onDropped: root.dropped(drop, model)
                }

                Kube.Label {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        right: parent.right
                    }
                    text: styleData.value
                    elide: Qt.ElideRight
                    color: Kube.Colors.viewBackgroundColor
                }
            }

            backgroundColor: Kube.Colors.textColor
            highlightedTextColor: Kube.Colors.highlightedTextColor
        }
    }
}
