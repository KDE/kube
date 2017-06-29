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
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1
import QtQuick.Templates 2.1 as T


import org.kube.framework 1.0 as Kube

Kube.TextField {
    id: root

    property string searchTerm
    property variant model

    selectByMouse: true

    onTextChanged: {
        if (text.length >= 2) {
            searchTerm = text
            startCompleting()
        } else {
            searchTerm = ""
            abort()
        }
    }
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
        accept()
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
        root.accepted();
    }

    function abort() {
        popup.close()
    }

    Controls2.Popup {
        id: popup
        x: 0
        y: root.height
        padding: 0
        width: root.width
        contentHeight: rect.height

        Rectangle {
            id: rect

            anchors.top: popup.top
            anchors.left: popup.left

            height: listView.contentHeight
            width: popup.width

            border.color: Kube.Colors.textColor
            color: Kube.Colors.backgroundColor

            radius: 5
            ListView {
                id: listView
                height: contentHeight
                width: parent.width
                interactive: true
                model: root.model
                //TODO abstract listItem
                delegate: T.ItemDelegate {
                    id: listDelegate

                    width: listView.width
                    height: root.height
                    padding: Kube.Units.smallSpacing

                    text: model.text

                    checked: listView.currentIndex == index

                    onClicked:  {
                        listView.currentIndex = model.index
                        accept()
                    }

                    //Content
                    contentItem: Item {
                        width: parent.width - padding * 2
                        height: parent.height - padding * 2

                        Column {
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                right: parent.right
                            }

                            Kube.Label{
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                }
                                text: model.text
                                color: listDelegate.checked ? Kube.Colors.highlightedTextColor : Kube.Colors.textColor
                                elide: Text.ElideRight
                            }
                        }
                    }

                    background: Rectangle {
                        color: listDelegate.checked ? Kube.Colors.highlightColor : Kube.Colors.viewBackgroundColor

                        border.width: 1
                        border.color: Kube.Colors.buttonColor
                    }
                }
            }
        }
    }
}
