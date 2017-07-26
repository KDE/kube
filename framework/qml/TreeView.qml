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
FocusScope {
    id: root
    default property alias __columns: treeView.__columns
    property alias model: treeView.model
    property alias currentIndex: treeView.currentIndex
    /*
     * Because active focus is useless in list/treeviews we use the concept of an activeIndex.
     * The current selection represents the focused index. The activeIndex represents the selected index.
     */
    property var activeIndex: null
    signal dropped(var drop, var model)
    signal activated(var index)
    onActivated: {
        activeIndex = index
    }

    Flickable {
        id: flickableItem

        anchors.fill: parent

        Controls2.ScrollBar.vertical: Controls2.ScrollBar {}
        clip: true
        contentWidth: root.width
        contentHeight: treeView.implicitHeight
        Kube.ScrollHelper {
            id: scrollHelper
            flickable: flickableItem
        }

        TreeView {
            id: treeView

            anchors {
                left: parent.left
                right: parent.right
            }
            implicitHeight: __listView.contentItem.height + 2
            height: implicitHeight
            focus: true

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
                //TODO scroll view so the current index is always visible
            }

            function selectFirst()
            {
                treeView.selection.setCurrentIndex(model.index(0, 0), ItemSelectionModel.ClearAndSelect)
            }

            function selectNext()
            {
                //If we have already expanded children go to them instead
                var childIndex = model.index(0, 0, selection.currentIndex)
                if (childIndex.valid && treeView.isExpanded(selection.currentIndex)) {
                    treeView.selection.setCurrentIndex(childIndex, ItemSelectionModel.ClearAndSelect)
                } else {
                    //Otherwise just advance to the next index, if we can
                    var nextIndex = model.sibling(selection.currentIndex.row + 1, 0, selection.currentIndex)
                    if (nextIndex.valid) {
                        treeView.selection.setCurrentIndex(nextIndex, ItemSelectionModel.ClearAndSelect)
                    } else {
                        //Try to go to the next of the parent instead TODO do this recursively
                        var parentIndex = model.parent(selection.currentIndex)
                        if (parentIndex.valid) {
                            var parentNext = model.sibling(parentIndex.row + 1, 0, parentIndex)
                            treeView.selection.setCurrentIndex(parentNext, ItemSelectionModel.ClearAndSelect)
                        }
                    }
                }
            }

            function selectPrevious()
            {
                var previousIndex = model.sibling(selection.currentIndex.row - 1, 0, selection.currentIndex)
                if (previousIndex.valid) {
                    treeView.selection.setCurrentIndex(previousIndex, ItemSelectionModel.ClearAndSelect)
                    //TODO if the previous index is expanded, go to the last visible child instead (recursively)
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
            Keys.onRightPressed: treeView.expand(selection.currentIndex)
            Keys.onLeftPressed: treeView.collapse(selection.currentIndex)

            //Forward the signal because on a desktopsystem activated is only triggerd by double clicks
            onClicked: treeView.activated(index)

            onActivated: root.activated(index)

            alternatingRowColors: false
            headerVisible: false

            style: TreeViewStyle {
                rowDelegate: Rectangle {
                    //FIXME Uses internal API to get to the model index
                    property bool isActive: root.activeIndex === treeView.__model.mapRowToModelIndex(styleData.row)

                    height: Kube.Units.gridUnit * 1.5
                    //FIXME This is the only way I could find to get the correct width. parent.width is way to wide
                    width: parent.parent.parent ? parent.parent.parent.width : 0
                    color: Kube.Colors.textColor

                    Rectangle {
                        anchors.fill: parent
                        color: Kube.Colors.highlightColor
                        visible: isActive
                    }
                    Rectangle {
                        anchors.fill: parent
                        border.width: 2
                        border.color: Kube.Colors.focusedButtonColor
                        color: "transparent"
                        visible: styleData.selected
                    }
                }

                frame: Rectangle {
                    color: Kube.Colors.textColor
                }

                branchDelegate: Kube.Label {
                    width: 16
                    height: width
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: Kube.Units.smallSpacing

                    color: Kube.Colors.viewBackgroundColor
                    text: styleData.isExpanded ? "-" : "+"
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
                        font.underline: treeView.activeFocus && styleData.selected
                        color: Kube.Colors.viewBackgroundColor
                    }
                }

                backgroundColor: Kube.Colors.textColor
                highlightedTextColor: Kube.Colors.highlightedTextColor
            }
        }
    }
}
