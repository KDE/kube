/*
 *  Copyright (C) 2016 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import QtQuick.Layouts 1.1
import QtQuick.Templates 2.0 as T


import org.kube.framework 1.0 as Kube

Kube.TextField {
    id: root

    property string searchTerm
    property variant model

    signal aborted()

    selectByMouse: true

    onTextChanged: {
        if (text.length >= 2) {
            searchTerm = text
            startCompleting()
        } else {
            searchTerm = ""
            popup.close()
        }
    }

    onEditingFinished: {
        accept()
    }

    validator: RegExpValidator { regExp: /.+/ }

    Keys.onDownPressed: {
        listView.incrementCurrentIndex()
    }
    Keys.onUpPressed: {
        listView.decrementCurrentIndex()
    }
    Keys.onRightPressed: {
        startCompleting()
    }
    Keys.onTabPressed: {
        if (popup.visible) {
            listView.incrementCurrentIndex()
        } else {
            event.accepted = false
        }
    }
    Keys.onReturnPressed: {
        if (acceptableInput) {
            accept()
        } else {
            abort()
        }
    }
    Keys.onEscapePressed: {
        abort()
    }

    function startCompleting() {
        if (!popup.visible) {
            popup.open()
            listView.currentIndex = -1
        }
    }

    function accept() {
        if (listView.currentItem) {
            root.text = listView.currentItem.text;
        }
        popup.close()
        root.accepted()
    }

    function abort() {
        popup.close()
        root.aborted()
    }

    Popup {
        id: popup
        x: 0
        y: root.height
        padding: 0
        width: root.width
        contentHeight: listView.height
        height: contentHeight

        Kube.ListView {
            id: listView
            contentHeight: model.count * (root.height + spacing)
            height: Math.min(contentHeight, Kube.Units.gridUnit * 20)
            width: parent.width
            interactive: true
            model: root.model
            delegate: Kube.ListDelegate {
                id: listDelegate

                height: root.height
                padding: Kube.Units.smallSpacing

                text: model.text

                contentItem: Item {
                    width: parent.width - padding * 2
                    height: parent.height - padding * 2
                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            listView.currentIndex = index
                            accept()
                        }
                    }
                    Kube.Label {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            right: parent.right
                        }
                        text: model.text
                        color: listDelegate.textColor
                        elide: Text.ElideRight
                        ToolTip.visible: mouseArea.containsMouse
                        ToolTip.text: text
                    }
                }
            }
        }
    }
}
