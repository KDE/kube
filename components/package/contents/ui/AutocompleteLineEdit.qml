/*
  Copyright (C) 2016 Christian Mollekopf, <mollekopf@kolabsys.com>

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

import QtQuick 2.7
import QtQuick.Controls 1.4 as Controls1
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0

import org.kde.kirigami 1.0 as Kirigami

Controls2.TextField {
    id: textField

    property string searchTerm
    property variant model
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
        textField.text = listView.currentItem.text;
        popup.close()
    }

    function abort() {
        popup.close()
    }

    Controls2.Popup {
        id: popup
        x: 0
        y: textField.y + textField.height
        padding: 0
        contentWidth: rect.width
        contentHeight: rect.height

        Rectangle {
            color: Kirigami.Theme.backgroundColor
            id: rect
            anchors.top: popup.top
            anchors.left: popup.left
            height: listView.contentHeight
            width: textField.width
            border.color: "Black"
            radius: 5
            Controls1.ScrollView {
                id: scrollView
                anchors.fill: parent
                ListView {
                    id: listView
                    height: childrenRect.height
                    width: scrollView.width
                    interactive: true
                    model: textField.model
                    delegate: Kirigami.AbstractListItem {
                        id: listDelegate
                        property string text: model.text

                        width: listView.width
                        height: textField.height

                        enabled: true
                        supportsMouseEvents: true

                        checked: listView.currentIndex == index
                        onClicked:  {
                            listView.currentIndex = model.index
                            accept()
                        }

                        //Content
                        Item {
                            width: parent.width
                            height: parent.height

                            Column {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    left: parent.left
                                }

                                Text{
                                    text: model.text
                                    color: listDelegate.checked ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
