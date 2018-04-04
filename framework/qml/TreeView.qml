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
import QtQuick.Controls 1.4 as Controls1
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
     * FIXME: This is what QItemSelectionModel selection vs current selection are for. Try to use that instead.
     */
    property var activeIndex: null
    signal activated(var index)
    onActivated: {
        activeIndex = index
    }

    function indexFromRow(row) {
        //FIXME Uses internal API to get to the model index
        return treeView.__model.mapRowToModelIndex(row)
    }

    Flickable {
        id: flickableItem

        anchors.fill: parent

        Controls2.ScrollBar.vertical: Kube.ScrollBar { invertedColors: true }
        clip: true
        contentWidth: root.width
        contentHeight: treeView.implicitHeight
        Kube.ScrollHelper {
            id: scrollHelper
            flickable: flickableItem
        }

        Controls1.TreeView {
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


            onActiveFocusChanged: {
                //Set an initially focused item when the list view receives focus
                if (activeFocus) {
                    //If there is a selected index, reset to selected index.
                    if (root.activeIndex) {
                        treeView.selection.setCurrentIndex(root.activeIndex, ItemSelectionModel.Current)
                    } else {
                        treeView.selection.setCurrentIndex(model.index(0, 0), ItemSelectionModel.ClearAndSelect)
                    }
                } else {
                    selection.clearCurrentIndex()
                }
            }

            Keys.onReturnPressed: treeView.activated(selection.currentIndex)

            //Forward the signal because on a desktopsystem activated is only triggerd by double clicks
            onClicked: treeView.activated(index)

            onActivated: root.activated(index)

            alternatingRowColors: false
            headerVisible: false

            style: TreeViewStyle {
                rowDelegate: Controls2.Control {
                    id: delegateRoot
                    property bool isActive: root.activeIndex === indexFromRow(styleData.row)
                    height: Kube.Units.gridUnit * 1.5
                    //FIXME This is the only way I could find to get the correct width. parent.width is way to wide
                    width: parent.parent.parent ? parent.parent.parent.width : 0
                    focus: false
                    hoverEnabled: true
                    Kube.DelegateBackground {
                        anchors.fill: parent
                        color: Kube.Colors.textColor
                        focused: styleData.selected || delegateRoot.hovered
                        selected: isActive
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

                backgroundColor: Kube.Colors.textColor
                highlightedTextColor: Kube.Colors.highlightedTextColor
            }
        }
    }
}
