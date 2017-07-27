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

    Kube.Listener {
        filter: Kube.Messages.mailSelection
        onMessageReceived: {
            root.mail = message.mail
            listView.forceLayout()
        }
    }

    Kube.Listener {
        filter: Kube.Messages.folderSelection
        onMessageReceived: {
            root.hideTrash = !message.trash
            root.hideNonTrash = message.trash
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Kube.Colors.backgroundColor

        Kube.ListView {
            id: listView
            focus: true

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            //Shrink the listview if the content doesn't fill the full height, so the email appears on top instead of on the bottom.
            height: Math.min(contentHeight, parent.height)

            verticalLayoutDirection: ListView.BottomToTop

            onActiveFocusChanged: {
                if (activeFocus) {
                    if (currentIndex < 0) {
                        currentIndex = 0
                    }
                } else {
                    currentIndex = -1
                }
            }

            //Position view so the last email begins on top of the view
            onContentHeightChanged: {
                //FIXME This doesn't work quite correctly when we have headers and footers in the listview and the mail loads to slowly and only one item to show.
                listView.positionViewAtIndex(0, ListView.End)
                //A futile attempt to fix the problem above
                listView.returnToBounds()
            }

            Keys.onDownPressed: {
                decrementCurrentIndex()
                positionViewAtIndex(listView.currentIndex, ListView.End)
            }

            Keys.onUpPressed: {
                incrementCurrentIndex()
                positionViewAtIndex(listView.currentIndex, ListView.End)
            }

            onCurrentIndexChanged: {
                markAsReadTimer.restart();
            }

            model: Kube.MailListModel {
                mail: root.mail
            }

            header: Item {
                height: Kube.Units.gridUnit * 0.5
                width: parent.width
            }

            footer: Item {
                height: Kube.Units.gridUnit
                width: parent.width
            }

            delegate: FocusScope {
                id: delegateRoot

                property var currentData: model

                focus: false
                activeFocusOnTab: false

                height: sheet.height + Kube.Units.gridUnit
                width: parent.width
                visible: !((root.hideTrash && model.trash) || (root.hideNonTrash && !model.trash))

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: delegateRoot.ListView.view.currentIndex = index
                    onClicked: delegateRoot.ListView.view.currentIndex = index
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
                    trash: model.trash
                    draft: model.draft
                    sent: model.sent
                    incomplete: model.incomplete
                    current: delegateRoot.ListView.isCurrentItem
                }
            }


            //Optimize for view quality
            pixelAligned: true


            //The cacheBuffer needs to be large enough to fit the whole thread.
            //Otherwise the contentHeight will constantly increase and decrease,
            //which will break lot's of things.
            cacheBuffer: 100000

            Timer {
                id: markAsReadTimer
                interval: 2000
                running: false
                repeat: false
                onTriggered: {
                    if (listView.currentItem && !!listView.currentItem.currentData.mail) {
                        Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": listView.currentItem.currentData.mail})
                    }
                }
            }

            //Intercept all scroll events,
            //necessary due to the webengineview
            Kube.MouseProxy {
                anchors.fill: parent
                target: listView.mouseProxy
                forwardWheelEvents: true
            }
        }
    }
}
