/*
 * Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 * Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Controls 1.4 as Controls1
import QtQuick.Controls 2
import QtQuick.Layouts 1.1

import org.kube.components.mailviewer 1.0 as MV
import org.kube.framework 1.0 as Kube

import org.kde.kirigami 1.0 as Kirigami

Rectangle {
    id: root

    property variant message;
    property variant subject;
    property variant sender;
    property variant senderName;
    property variant to;
    property variant cc;
    property variant bcc;
    property variant date;
    property variant trash;
    property variant draft;
    property variant sent;
    property bool incomplete: false;
    property bool current: false;

    implicitHeight: header.height + attachments.height + body.height + incompleteBody.height + footer.height + Kube.Units.largeSpacing

    Shortcut {
        sequence: "V"
        onActivated: debugPopup.open()
        enabled: root.current
    }

    //Overlay for non-active mails
    Rectangle {
        anchors.fill: parent
        visible: !current
        color: "lightGrey"
        z: 1
        opacity: 0.2
    }

    color: Kube.Colors.viewBackgroundColor

    Kube.MessageParser {
        id: messageParser
        message: root.message
    }

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

            text: Qt.formatDateTime(root.date, "dd MMM yyyy hh:mm")

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

                    text: root.senderName
                    font.weight: Font.DemiBold
                    opacity: 0.75
                }

                Kube.Label {
                    width: parent.width - senderName.width - date_label.width - Kube.Units.largeSpacing


                    text: root.sender
                    elide: Text.ElideRight
                    opacity: 0.75
                    clip: true
                }
            }

            Kube.Label {
                id: subject

                width: to.width

                text: root.subject
                elide: Text.ElideRight
                opacity: 0.75
                font.italic: true
                states: [
                    State {
                        name: "trash"; when: root.trash
                        PropertyChanges { target: subject; text: "Trash: " + root.subject }
                    },
                    State {
                        name: "draft"; when: root.draft
                        PropertyChanges { target: subject; text: "Draft: " + root.subject }
                    },
                    State {
                        name: "sent"; when: root.sent
                        PropertyChanges { target: subject; text: "Sent: " + root.subject }
                    }
                ]
            }

            Kube.Label {
                id: recipients

                width: parent.width - goDown.width - Kube.Units.smallSpacing

                text:"to: "+ root.to + " "  + root.cc + " " +  root.bcc
                elide: Text.ElideRight
                opacity: 0.75
            }

            Kube.Label {
                id: to

                width: parent.width - goDown.width - Kube.Units.smallSpacing

                text:"to: " + root.to
                wrapMode: Text.WordWrap
                opacity: 0.75
            }

            Kube.Label {
                id: cc

                width: parent.width - goDown.width - Kube.Units.smallSpacing

                text:"cc: " + root.cc
                wrapMode: Text.WordWrap
                opacity: 0.75
            }

            Kube.Label {
                id: bcc

                width: parent.width - goDown.width - Kube.Units.smallSpacing

                text:"bcc: " + root.bcc
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

            Kube.IconButton {
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
            model: messageParser.attachments

            delegate: AttachmentDelegate {
                name: model.name
                icon: model.iconName

                clip: true
                onDownload: {
                    messageParser.attachments.saveAttachmentToDisk(messageParser.attachments.index(index, 0))
                }
                onOpen: {
                    messageParser.attachments.openAttachment(messageParser.attachments.index(index, 0))
                }
            }
        }
    }

    Item {
        id: body

        visible: !root.incomplete
        anchors {
            top: header.bottom
            left: header.left
            right: header.right
            leftMargin: Kube.Units.largeSpacing
            rightMargin: Kube.Units.largeSpacing
            topMargin: Math.max(attachments.height, Kube.Units.largeSpacing)
        }
        height: mailViewer.height + 20

        clip: true

        MV.MailViewer {
            id: mailViewer
            anchors.top: body.top
            anchors.left: body.left
            anchors.right: body.right
            model: messageParser.newTree
            debug: false
        }

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
        visible: root.incomplete
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

        Kube.IconButton {
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

    Kube.Popup {
        id: debugPopup
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        x: 0
        y: 0
        width: root.width
        height: root.height

        Flickable {
            id: flickable
            anchors.fill: parent
            contentHeight: content.height
            contentWidth: parent.width
            Column {
                id: content
                width: flickable.width
                height: childrenRect.height

                Controls1.TreeView {
                    id: mailStructure
                    width: parent.width
                    height: implicitHeight
                    Controls1.TableViewColumn {
                        role: "type"
                        title: "Type"
                    }
                    Controls1.TableViewColumn {
                        role: "embeded"
                        title: "Embeded"
                    }
                    Controls1.TableViewColumn {
                        role: "securityLevel"
                        title: "SecurityLevel"
                    }
                    Controls1.TableViewColumn {
                        role: "content"
                        title: "Content"
                    }
                    model: messageParser.newTree
                    itemDelegate: Item {
                        property variant currentData: styleData.value
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            color: styleData.textColor
                            elide: styleData.elideMode
                            text: styleData.value
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                textEdit.text = styleData.value
                            }
                        }
                    }
                }
                Controls1.TreeView {
                    id: attachmentsTree
                    width: parent.width
                    height: implicitHeight
                    Controls1.TableViewColumn {
                        role: "type"
                        title: "Type"
                    }
                    Controls1.TableViewColumn {
                        role: "name"
                        title: "Name"
                    }
                    Controls1.TableViewColumn {
                        role: "size"
                        title: "Size"
                    }
                    model: messageParser.attachments
                }
                TextEdit {
                    id: textEdit
                    width: parent.width
                    readOnly: true
                    selectByMouse: true
                    textFormat: TextEdit.PlainText
                    wrapMode: TextEdit.Wrap
                    height: implicitHeight
                }
                TextEdit {
                    id: rawContent
                    width: parent.width
                    readOnly: true
                    selectByMouse: true
                    textFormat: TextEdit.PlainText
                    wrapMode: TextEdit.Wrap
                    height: implicitHeight
                    text: messageParser.rawContent
                }
            }
        }
    }
}
