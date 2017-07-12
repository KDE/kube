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


Rectangle {
    id: root

    property variant mail;
    property int currentIndex: 0;
    property bool scrollToEnd: true;
    property variant currentMail: null;
    property bool hideTrash: true;
    property bool hideNonTrash: false;

    Kube.Listener {
        filter: Kube.Messages.mailSelection
        onMessageReceived: {
            root.mail = message.mail
        }
    }

    Kube.Listener {
        filter: Kube.Messages.folderSelection
        onMessageReceived: {
            root.hideTrash = !message.trash
            root.hideNonTrash = message.trash
        }
    }

    onCurrentIndexChanged: {
        markAsReadTimer.restart();
    }
    onMailChanged: {
        scrollToEnd = true;
        currentMail = null;
    }

    color: Kube.Colors.backgroundColor

    Kube.ListView {
        id: listView

        anchors.fill: parent

        verticalLayoutDirection: ListView.BottomToTop

        function setCurrentIndex()
        {
            /**
             * This will detect the index at the "scrollbar-position" (visibleArea.yPosition).
             * This ensures that the first and last entry can become the currentIndex,
             * but in the middle of the list the item in the middle is set as the current item.
             */
            var yPos = 0.5;
            if (listView.visibleArea.yPosition < 0.4) {
                yPos = 0.2 + (0.2 * listView.visibleArea.yPosition);
            }
            if (listView.visibleArea.yPosition > 0.6) {
                yPos = 0.6 + (0.2 * listView.visibleArea.yPosition)
            }
            var indexAtCenter = listView.indexAt(root.width / 2, contentY + root.height * yPos);
            if (indexAtCenter >= 0) {
                root.currentIndex = indexAtCenter;
            } else {
                root.currentIndex = count - 1;
            }
        }

        clip: true

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

        delegate: mailDelegate

        //Setting the currentIndex results in further lags. So we don't do that either.
        // currentIndex: root.currentIndex

        //Optimize for view quality
        pixelAligned: true

        onContentYChanged: {
            //We have to track our current mail manually
            setCurrentIndex();
        }

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
                if (!!root.currentMail) {
                    Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": root.currentMail})
                }
            }
        }

        //Intercept all scroll events,
        //necessary due to the webengineview
        Kube.MouseProxy {
            anchors.fill: parent
            target: listView
            forwardWheelEvents: true
        }
    }
    Component {
        id: mailDelegate

        Item {
            id: wrapper
            property bool isCurrent: root.currentIndex === index;
            onIsCurrentChanged: {
                if (isCurrent) {
                    root.currentMail = model.mail
                }
            }

            height: sheet.height + Kube.Units.gridUnit
            width: parent.width
            visible: !((root.hideTrash && model.trash) || (root.hideNonTrash && !model.trash))

            MouseArea {
                anchors.fill: parent
                enabled: parent.enabled
                hoverEnabled: true
                onEntered: root.currentIndex = index
                onClicked: root.currentIndex = index
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
                current: isCurrent
            }
        }
    }
}
