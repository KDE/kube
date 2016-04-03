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

Kirigami.ApplicationWindow {
    id: app

    //FIXME remove fixed pixel hight
    //for now just convinience during testing
    width: 1080 / 2.7
    height: (1920 - 40)/ 2.7

    visible: true

    globalDrawer: Kirigami.GlobalDrawer {
        title: "Kube Mail"
        titleIcon: "kmail"

        actions: [
            Kirigami.Action {
                text: "Unread"
                onTriggered: {


                    pageStack.initialPage = Qt.resolvedUrl("MailListPage.qml");
                }
            },
            Kirigami.Action {
                text: "Imporant"
            },
            Kirigami.Action {
                text: "Drafts"
            },
            Kirigami.Action {
                text: "All Mail"
            }
        ]

        ListView {
            id: listView

            anchors.fill: parent

            model: FolderListModel {}

            delegate: Kirigami.BasicListItem {

                label: model.name

                enabled: true
            }
        }
    }

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: MailListPage {
        title: "Inbox"
    }
}
