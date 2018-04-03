/*
 *  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsystems.com>
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
import QtQuick.Controls 2
import QtQuick.Layouts 1.1
import org.kube.framework 1.0 as Kube

import QtQml 2.2 as QtQml


FocusScope {
    id: root

    property variant mail;
    property bool hideTrash: true;
    property bool hideNonTrash: false;
    property string searchString: ""

    Kube.Listener {
        filter: Kube.Messages.searchString
        onMessageReceived: root.searchString = message.searchString
    }

    Rectangle {
        anchors.fill: parent
        color: Kube.Colors.backgroundColor

        Kube.ConversationListView {
            id: listView
            objectName: "listView"
            focus: true

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            //Shrink the listview if the content doesn't fill the full height, so the email appears on top instead of on the bottom.
            height: Math.min(contentHeight, parent.height)

            model: Kube.MailListModel {
                mail: root.mail
            }

            Keys.onPressed: {
                if (event.text == "j" || event.matches(StandardKey.MoveToNextLine)) {
                    listView.incrementCurrentIndex()
                } else if (event.text == "k" || event.matches(StandardKey.MoveToPreviousLine)) {
                    listView.decrementCurrentIndex()
                } else if (event.text == "d") {
                   //Not implemented as a shortcut because we want it only to apply if we have the focus
                   Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": listView.currentItem.currentData.mail})
                }
            }


            delegate: FocusScope {
                id: delegateRoot

                property var currentData: model
                property bool isCurrentItem: false
                property int index: -1

                focus: true
                activeFocusOnTab: false
                onActiveFocusChanged: {
                    if (activeFocus) {
                        listView.currentIndex = delegateRoot.index
                    }
                }

                height: sheet.height + Kube.Units.gridUnit
                width: listView.width
                //FIXME breaks keyboard navigation because we don't jump over invisible items
                visible: !((root.hideTrash && model.trash) || (root.hideNonTrash && !model.trash))

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    hoverEnabled: true
                    onEntered: delegateRoot.forceActiveFocus(Qt.MouseFocusReason)
                }

                MailViewer {
                    id: sheet
                    anchors.centerIn: parent
                    width: parent.width - Kube.Units.gridUnit * 2

                    message: model.mimeMessage
                    subject: model.subject
                    sender: model.sender
                    senderName: model.senderName
                    to: model.to
                    cc: model.cc
                    bcc: model.bcc
                    date: model.date
                    unread: model.unread
                    trash: model.trash
                    draft: model.draft
                    sent: model.sent
                    incomplete: model.incomplete
                    current: delegateRoot.isCurrentItem
                    searchString: root.searchString
                }
            }

        }
    }
}
