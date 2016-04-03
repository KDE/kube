/*
 * Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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
import QtQuick.Layouts 1.2

import org.kde.kirigami 1.0 as Kirigami

Kirigami.ScrollablePage {
    id: root

    anchors.fill: parent

    background: Rectangle {
        color: Kirigami.Theme.viewBackgroundColor
    }

    mainAction: Kirigami.Action {
        iconName: "mail-message-new"

        onTriggered: {
            app.pageStack.push(Qt.resolvedUrl("ComposerPage.qml"))
        }

    }

    ListView {
        id: listView

        model: MailListModel {}

        delegate: Kirigami.SwipeListItem {
            id: mailListDelegate

            property bool important: model.important
            property bool unread: model.unread

            actions: [
                Kirigami.Action {
                    iconName: "mail-mark-important"

                    onTriggered: {
                        if(important) {
                            important = false
                        } else {
                            important = true
                        }
                    }
                },
                Kirigami.Action {
                    iconName: "mail-mark-unread-new"

                    onTriggered: {
                        if(unread) {
                            unread = false
                        } else {
                            unread = true
                        }
                    }
                },
                Kirigami.Action {
                    iconName: "entry-delete"
                }
            ]

            enabled: true

            onClicked: {
                app.pageStack.push(Qt.resolvedUrl("SingleMailPage.qml"))
            }

            RowLayout {
                anchors.fill: parent

                width: parent.width

                Avatar {
                    id: avatar

                    height: textItem.height * 0.9
                    width: height

                    name: model.sender
                }

                ColumnLayout {
                    id: textItem

                    Controls.Label {
                        text: model.subject

                        color: important ? "#da4453" : unread ? "#1d99f3" : Kirigami.Theme.textColor
                    }

                    Controls.Label {
                        text: model.sender
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
