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

import QtQuick 2
import QtQuick.Controls 2
import QtQuick.Layouts 1

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    property var model: null
    property var currentIndex: null
    property alias count: listView.count

    signal dropped(var index, var drop)

    function clearSelection() {
        listView.currentIndex = -1
    }

    function selectRootIndex() {
        if (listView.count >= 1) {
            listView.currentIndex = 0
        }
    }

    function selectNext() {
        listView.incrementCurrentIndex()
    }

    function selectPrevious() {
        listView.decrementCurrentIndex()
    }

    Kube.ListView {
        id: listView

        anchors.fill: parent
        focus: true

        model: Kube.TreeModelAdaptor {
            id: modelAdaptor
            model: root.model
        }

        onCurrentIndexChanged: {
            root.currentIndex = modelAdaptor.mapRowToModelIndex(listView.currentIndex)
        }

        ScrollBar.vertical: Kube.ScrollBar { invertedColors: true }

        delegate: Kube.ListDelegate {
            id: delegate
            width: listView.availableWidth
            height: Kube.Units.gridUnit * 1.5
            hoverEnabled: true
            property bool isActive: listView.currentIndex === index

            DropArea {
                anchors.fill: parent

                Rectangle {
                    anchors.fill: parent
                    color: Kube.Colors.viewBackgroundColor
                    opacity: 0.3
                    visible: parent.containsDrag
                }

                onDropped: root.dropped(modelAdaptor.mapRowToModelIndex(index), drop)
            }

            background: Kube.DelegateBackground {
                anchors.fill: parent
                color: Kube.Colors.textColor
                focused: delegate.activeFocus || delegate.hovered
                selected: isActive
            }

            function toggleExpanded() {
                var idx = model._q_TreeView_ModelIndex
                if (modelAdaptor.isExpanded(idx)) {
                    modelAdaptor.collapse(idx)
                } else {
                    modelAdaptor.expand(idx)
                }
            }

            Keys.onSpacePressed: toggleExpanded()

            RowLayout {
                anchors {
                    fill: parent
                    leftMargin: 2 + (model._q_TreeView_ItemDepth + 1) * Kube.Units.largeSpacing
                }
                spacing: Kube.Units.smallSpacing
                Kube.Label {
                    id: label
                    Layout.fillWidth: true
                    text: model.name
                    color: Kube.Colors.highlightedTextColor
                    elide: Text.ElideLeft
                    clip: false

                    Kube.IconButton {
                        anchors {
                            right: label.left
                            verticalCenter: label.verticalCenter
                        }
                        visible: model._q_TreeView_HasChildren
                        iconName: model._q_TreeView_ItemExpanded ? Kube.Icons.goDown_inverted : Kube.Icons.goNext_inverted
                        padding: 0
                        width: Kube.Units.gridUnit
                        height: Kube.Units.gridUnit
                        onClicked: toggleExpanded()
                        activeFocusOnTab: false
                        hoverEnabled: false
                    }
                }
            }
        }
    }
}
