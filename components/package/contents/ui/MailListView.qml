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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.1
import QtQml 2.2 as QtQml

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework

Controls.ScrollView {
    id: root
    property variant parentFolder
    property variant currentMail
    property bool isDraft : false

    onParentFolderChanged: {
        currentMail = null
    }

    ListView {
        id: listView

        model: KubeFramework.MailListModel {
            parentFolder: root.parentFolder
        }

        Keys.onDownPressed: {
            incrementCurrentIndex()
        }
        Keys.onUpPressed: {
            decrementCurrentIndex()
        }
        focus: true

        delegate: Kirigami.AbstractListItem {
            id: mailListDelegate

            width: listView.width
            height: Kirigami.Units.gridUnit * 4

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
                        color: mailListDelegate.checked ? Kirigami.Theme.textColor : model.unread ? "#1d99f3" : Kirigami.Theme.textColor
                    }

                    Text {
                        text: model.senderName
                        font.italic: true
                        color:  Kirigami.Theme.textColor
                    }
                }

                Text {
                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                    }
                    text: Qt.formatDateTime(model.date)
                    font.italic: true
                    color:  Kirigami.Theme.textColor
                    opacity: 0.5
                    font.pointSize: 9
                }

                Rectangle {

                    anchors {
                        right: parent.right
                    }

                    height: Kirigami.Units.gridUnit * 1.2
                    width: height

                    visible: !mailListDelegate.checked

                    radius: 80
                    color: model.unread ? "#1d99f3" : "lightgrey"

                    Text {
                        anchors.centerIn: parent

                        text: model.threadSize
                        color: "white"
                    }

                }
            }
            /*
             *                        Text{
             *                            text: model.subject
             *
             *                            color: mailListDelegate.checked ? Kirigami.Theme.textColor : model.unread ? "#1d99f3" : Kirigami.Theme.textColor
             *                            font.weight: model.unread || model.important ? Font.DemiBold : Font.Normal
        }

        Text {
        text: model.senderName

        color:  Kirigami.Theme.textColor
        }

        Text {
        text: Qt.formatDateTime(model.date)

        font.weight: Font.Light
        opacity: 0.5
        color: Kirigami.Theme.textColor
        }
        }
        Text {
        text: model.threadSize

        font.weight: Font.Light
        color:  Kirigami.Theme.textColor
        }

        }
        */
        }
    }
}
