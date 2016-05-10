/*
 * Copyright (C) 2015 Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.1
import QtQml 2.2 as QtQml

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.theme 1.0

Controls.ScrollView {
    id: root
    property variant parentFolder
    property variant currentMail
    property bool isDraft : false

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

            enabled: true
            supportsMouseEvents: true

            checked: listView.currentIndex == index
            onClicked:  {
                listView.currentIndex = model.index
            }

            //Content
            Item {
                width: parent.width
                height: Kirigami.Units.gridUnit * 4

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

                RowLayout {

                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                        left: parent.left
                    }

                    Avatar {
                        id: avatar

                        height: textItem.height
                        width: height

                        name: model.senderName
                    }

                    ColumnLayout {
                        id: textItem
                        height: Kirigami.Units.gridUnit * 3

                        Text{
                            text: model.subject

                            renderType: Text.NativeRendering
                            color: mailListDelegate.checked ? Kirigami.Theme.textColor : model.unread ? "#1d99f3" : Kirigami.Theme.textColor
                            font.weight: model.unread || model.important ? Font.DemiBold : Font.Normal
                        }

                        Text {
                            text: model.senderName

                            renderType: Text.NativeRendering
                            color:  Kirigami.Theme.textColor
                        }

                        Text {
                            text: Qt.formatDateTime(model.date)

                            renderType: Text.NativeRendering
                            font.weight: Font.Light
                            opacity: 0.5
                            color: Kirigami.Theme.textColor
                        }
                    }
                }

                Rectangle {

                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                        bottomMargin: 5
                    }

                    color: "lightgrey" //TODO wait for Kirigami pallete update

                    height: Kirigami.Units.gridUnit * 2
                    width: height

                    visible: mailListDelegate.checked ? false : model.unread

                    radius: 100

                    Text {
                        anchors.centerIn: parent
                        text: "+1" //TODO wait for thread implementation
                        color: Kirigami.Theme.complementaryTextColor
                        font.weight: Font.DemiBold
                    }
                }
            }
        }
    }
}
