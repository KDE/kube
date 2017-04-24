/*
 *  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
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
import QtQuick.Controls 1.3 as Controls1
import QtQuick.Controls 2
import QtQuick.Layouts 1.1
import org.kde.kirigami 1.0 as Kirigami
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

    ListView {
        id: listView
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

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
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

        boundsBehavior: Flickable.StopAtBounds

        //default is 1500, which is not usable with a mouse
        flickDeceleration: 10000

        //Optimize for view quality
        pixelAligned: true

        Timer {
            id: scrollToEndTimer
            interval: 10
            running: false
            repeat: false
            onTriggered: {
                //Only do this once per conversation
                root.scrollToEnd = false;
                root.currentIndex = listView.count - 1
                //positionViewAtEnd/Index don't work
                listView.contentY = Math.max(listView.contentHeight - listView.height, 0)
            }
        }

        onCountChanged: {
            if (root.scrollToEnd) {
                scrollToEndTimer.restart()
            }
        }

        onContentHeightChanged: {
            //Initially it will resize a lot, so we keep waiting
            if (root.scrollToEnd) {
                scrollToEndTimer.restart()
            }
        }

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
                Kube.Fabric.postMessage(Kube.Messages.markAsRead, {"mail": root.currentMail})
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
                onEntered: {
                    root.currentIndex = index
                }
                onClicked: {
                    root.currentIndex = index
                }
            }

            Rectangle {
                id: sheet
                anchors.centerIn: parent
                implicitHeight: header.height + attachments.height + body.height + incompleteBody.height + footer.height + Kube.Units.largeSpacing
                width: parent.width - Kube.Units.gridUnit * 2

                //Overlay for non-active mails
                Rectangle {
                    anchors.fill: parent
                    visible: !wrapper.isCurrent
                    color: "lightGrey"
                    z: 1
                    opacity: 0.2
                }

                color: Kube.Colors.viewBackgroundColor

                //BEGIN header
                Item {
                    id: header

                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        margins: Kube.Units.largeSpacing
                    }

                    height: headerContent.height + Kube.Units.smallSpacing

                    states: [
                        State {
                            name: "small"
                            PropertyChanges { target: subject; wrapMode: Text.NoWrap}
                            PropertyChanges { target: recipients; visible: true}
                            PropertyChanges { target: to; visible: false}
                            PropertyChanges { target: cc; visible: false}
                            PropertyChanges { target: bcc; visible: false}
                        },
                        State {
                            name: "details"
                            PropertyChanges { target: subject; wrapMode: Text.WrapAnywhere}
                            PropertyChanges { target: recipients; visible: false}
                            PropertyChanges { target: to; visible: true}
                            PropertyChanges { target: cc; visible: true}
                            PropertyChanges { target: bcc; visible: true}
                        }
                    ]

                    state: "small"

                    Kube.Label {
                        id: date_label

                        anchors {
                            right: seperator.right
                            top: parent.top
                        }

                        text: Qt.formatDateTime(model.date, "dd MMM yyyy hh:mm")

                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.7
                        opacity: 0.75
                    }

                    Column {
                        id: headerContent

                        anchors {
                            //left: to_l.right
                            horizontalCenter: parent.horizontalCenter
                        }

                        //spacing: Kube.Units.smallSpacing

                        width: parent.width

                        Row{
                            id: from

                            width: parent.width

                            spacing: Kube.Units.smallSpacing
                            clip: true

                            Kube.Label {
                                id: senderName

                                text: model.senderName
                                font.weight: Font.DemiBold
                                opacity: 0.75
                            }

                            Kube.Label {
                                width: parent.width - senderName.width - date_label.width - Kube.Units.largeSpacing


                                text: model.sender
                                elide: Text.ElideRight
                                opacity: 0.75
                                clip: true
                            }
                        }

                        Kube.Label {
                            id: subject

                            width: to.width

                            text: model.subject
                            elide: Text.ElideRight
                            opacity: 0.75
                            font.italic: true
                            states: [
                                State {
                                    name: "trash"; when: model.trash
                                    PropertyChanges { target: subject; text: "Trash: " + model.subject }
                                },
                                State {
                                    name: "draft"; when: model.draft
                                    PropertyChanges { target: subject; text: "Draft: " + model.subject }
                                },
                                State {
                                    name: "sent"; when: model.sent
                                    PropertyChanges { target: subject; text: "Sent: " + model.subject }
                                }
                            ]
                        }

                        Kube.Label {
                            id: recipients

                            width: parent.width - goDown.width - Kube.Units.smallSpacing

                            text:"to: "+ model.to + " "  + model.cc + " " +  model.bcc
                            elide: Text.ElideRight
                            opacity: 0.75
                        }

                        Kube.Label {
                            id: to

                            width: parent.width - goDown.width - Kube.Units.smallSpacing

                            text:"to: " + model.to
                            wrapMode: Text.WordWrap
                            opacity: 0.75
                        }

                        Kube.Label {
                            id: cc

                            width: parent.width - goDown.width - Kube.Units.smallSpacing

                            text:"cc: " + model.cc
                            wrapMode: Text.WordWrap
                            opacity: 0.75
                        }

                        Kube.Label {
                            id: bcc

                            width: parent.width - goDown.width - Kube.Units.smallSpacing

                            text:"bcc: " + model.bcc
                            wrapMode: Text.WordWrap
                            opacity: 0.75
                        }

                    }

                    Rectangle {
                        id: goDown

                        anchors {
                            bottom: seperator.top
                            right: seperator.right
                        }

                        height: Kube.Units.gridUnit
                        width: height

                        color: Kube.Colors.backgroundColor

                        Controls1.ToolButton {
                            anchors.fill: parent

                            iconName: header.state === "details" ? Kube.Icons.goUp : Kube.Icons.goDown

                            onClicked: {
                                header.state === "details" ? header.state = "small" : header.state = "details"
                            }
                        }
                    }

                    Rectangle {
                        id: seperator

                        anchors {
                            left: parent.left
                            right: parent.right
                            bottom: parent.bottom
                        }

                        height: 1

                        color: Kube.Colors.textColor
                        opacity: 0.5
                    }
                }
                //END header

                Flow {
                    id: attachments

                    anchors {
                        top: header.bottom
                        topMargin: Kube.Units.smallSpacing
                        right: header.right
                    }

                    width: header.width - Kube.Units.largeSpacing

                    layoutDirection: Qt.RightToLeft
                    spacing: Kube.Units.smallSpacing
                    clip: true

                    Repeater {
                        model: body.attachments

                        delegate: AttachmentDelegate {
                            name: model.name
                            icon: "mail-attachment"

                            clip: true

                            //TODO size encrypted signed type
                        }
                    }
                }

                MailViewer {
                    id: body

                    anchors {
                        top: header.bottom
                        left: header.left
                        right: header.right
                        leftMargin: Kube.Units.largeSpacing
                        rightMargin: Kube.Units.largeSpacing
                        topMargin: Math.max(attachments.height, Kube.Units.largeSpacing)
                    }

                    width: header.width - Kube.Units.largeSpacing * 2
                    height: desiredHeight

                    message: model.mimeMessage
                    visible: !model.incomplete
                }

                Kube.Label {
                    id: incompleteBody
                    anchors {
                        top: header.bottom
                        left: header.left
                        right: header.right
                        leftMargin: Kube.Units.largeSpacing
                        rightMargin: Kube.Units.largeSpacing
                        topMargin: Math.max(attachments.height, Kube.Units.largeSpacing)
                    }
                    visible: model.incomplete
                    text: "Incomplete body..."
                    color: Kube.Colors.textColor
                    enabled: false
                    states: [
                        State {
                            name: "inprogress"; when: model.status == Kube.MailListModel.InProgressStatus
                            PropertyChanges { target: incompleteBody; text: "Downloading message..." }
                        },
                        State {
                            name: "error"; when: model.status == Kube.MailListModel.ErrorStatus
                            PropertyChanges { target: incompleteBody; text: "Failed to download message..." }
                        }
                    ]
                }
                Item {
                    id: footer

                    anchors.bottom: parent.bottom

                    height: Kube.Units.gridUnit * 2
                    width: parent.width

                    Kube.Label {
                        anchors{
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kube.Units.largeSpacing
                        }

                        text: model.trash ? qsTr("Delete Mail") : qsTr("Move to trash")
                        opacity: 0.5
                        MouseArea {
                            anchors.fill: parent
                            enabled: parent.enabled
                            onClicked: {
                                if (model.trash) {
                                    Kube.Fabric.postMessage(Kube.Messages.remove, {"mail": model.mail})
                                } else {
                                    Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": model.mail})
                                }
                            }
                        }
                    }

                    Controls1.ToolButton {
                        visible: !model.trash
                        anchors{
                            verticalCenter: parent.verticalCenter
                            right: parent.right
                            rightMargin: Kube.Units.largeSpacing
                        }

                        iconName: model.draft ? Kube.Icons.edit : Kube.Icons.replyToSender
                        onClicked: {
                            if (model.draft) {
                                Kube.Fabric.postMessage(Kube.Messages.edit, {"mail": model.mail, "isDraft": model.draft})
                            } else {
                                Kube.Fabric.postMessage(Kube.Messages.reply, {"mail": model.mail, "isDraft": model.draft})
                            }
                        }
                    }
                }
            }
        }
    }
}
