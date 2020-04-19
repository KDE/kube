/*
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    property variant currentMail: null

    Keys.onRightPressed: {
        modelIndexRetriever.increaseCurrentIndex()
    }
    Keys.onLeftPressed: {
        modelIndexRetriever.decreaseCurrentIndex()
    }
    Kube.ModelIndexRetriever {
        id: modelIndexRetriever
        model: Kube.MailListModel {
            filter: {
                "inbox": true,
                "headersOnly": true,
                "fetchMails": false
            }
        }
        currentIndex: 0
        onCurrentDataChanged: {
            root.currentMail = currentData.mail
        }
    }
    Column {
        anchors.fill: parent
        spacing: Kube.Units.smallSpacing
        Repeater {
            anchors {
                left: parent.left
                right: parent.right
            }
            model: Kube.MailListModel {
                filter: {
                    "mail": root.currentMail,
                    "headersOnly": false,
                    "fetchMails": true
                }
            }
            Kube.MailViewer {
                anchors {
                    left: parent.left
                    right: parent.right
                }
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
                current: true
            }
        }
        Row {
            spacing: Kube.Units.smallSpacing
            anchors {
                horizontalCenter: parent.horizontalCenter
            }
            height: Kube.Units.gridUnit
            Kube.Button {
                focus: true
                text: qsTr("Delete!")
                onClicked: {
                }
            }
            Kube.Button {
                text: qsTr("Reply!")
                onClicked: {
                }
            }
            Kube.Button {
                text: qsTr("Flag!")
                onClicked: {
                }
            }
            Kube.Button {
                text: qsTr("Previous")
                onClicked: {
                    modelIndexRetriever.decreaseCurrentIndex()
                }
            }
            Kube.Button {
                text: qsTr("Next")
                onClicked: {
                    modelIndexRetriever.increaseCurrentIndex()
                }
            }
        }
    }
}
