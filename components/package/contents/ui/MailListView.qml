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

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtQml 2.2 as QtQml

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework

Item {
    id: root
    property variant parentFolder
    property variant currentMail
    property bool isDraft : false
    property int currentIndex

    onParentFolderChanged: {
        currentMail = null
    }

    ListView {
        id: listView

        anchors.fill: parent

        focus: true

        ScrollBar.vertical: ScrollBar{
            id: scrollbar
        }

        //BEGIN keyboard nav
        Keys.onDownPressed: {
            incrementCurrentIndex()
        }
        Keys.onUpPressed: {
            decrementCurrentIndex()
        }
        //END keyboard nav

        currentIndex: root.currentIndex

        model: KubeFramework.MailListModel {
            parentFolder: root.parentFolder
        }

        delegate: Kirigami.AbstractListItem {
            id: mailListDelegate

            width: scrollbar.visible ? listView.width - scrollbar.width : listView.width
            height: Kirigami.Units.gridUnit * 4.5

            enabled: true
            supportsMouseEvents: true

            checked: listView.currentIndex == index
            onClicked:  {
                listView.currentIndex = model.index
            }

            //Content
            Item {
                width: parent.width
                height: parent.height

                QtQml.Binding {
                    target: root
                    property: "currentMail"
                    when: listView.currentIndex == index
                    value: model.domainObject
                }
                QtQml.Binding {
                    target: root
                    property: "isDraft"
                    when: listView.currentIndex == index
                    value: model.draft
                }

                //TODO implement bulk action
                //    Controls.CheckBox {
                //        visible: mailListDelegate.containsMouse == true
                //    }

                Column {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: Kirigami.Units.largeSpacing
                    }

                    Text{
                        text: model.subject
                        color: mailListDelegate.checked ? Kirigami.Theme.highlightedTextColor : model.unread ? "#1d99f3" : Kirigami.Theme.textColor

                        maximumLineCount: 2
                        width: mailListDelegate.width - Kirigami.Units.gridUnit * 3
                        wrapMode: Text.WrapAnywhere
                        elide: Text.ElideRight
                    }

                    Text {
                        text: model.senderName
                        font.italic: true
                        color: mailListDelegate.checked ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                        width: mailListDelegate.width - Kirigami.Units.gridUnit * 3
                        elide: Text.ElideRight
                    }
                }

                Text {
                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                    }
                    text: Qt.formatDateTime(model.date, "dd MMM yyyy")
                    font.italic: true
                    color: mailListDelegate.checked ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.disabledTextColor
                    font.pointSize: 9
                }


                Item {
                    anchors {
                        right: parent.right
                    }

                    height: Kirigami.Units.gridUnit * 1.2
                    width: height

                    visible: !mailListDelegate.checked

                    Rectangle {
                        anchors.fill: parent

                        opacity: model.unread ? 1 :  0.5
                        radius: 80
                        color: model.unread ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
                    }

                    Text {
                        anchors.centerIn: parent

                        text: model.threadSize
                        color: Kirigami.Theme.highlightedTextColor
                    }

                }
            }
        }
    }
}
