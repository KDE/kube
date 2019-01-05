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
    property bool unread;
    property alias searchString: mailViewer.searchString
    property alias autoLoadImages: mailViewer.autoLoadImages

    property bool collapsed: draft || sent

    implicitHeight: mainLayout.height + 2 * Kube.Units.largeSpacing

    Shortcut {
        sequence: "V"
        onActivated: debugPopupComponent.createObject(root).open()
        enabled: root.current
    }

    //highlight active mails
    border.width: current ? 1 : 0
    border.color: Kube.Colors.highlightColor

    color: Kube.Colors.viewBackgroundColor

    Kube.MessageParser {
        id: messageParser
        message: root.message
    }

    states: [
        State {
            name: "full"
        },
        State {
            name: "incomplete"; when: root.incomplete
            PropertyChanges { target: attachments; visible: false}
            PropertyChanges { target: body; visible: false}
            PropertyChanges { target: footer; visible: false}
            PropertyChanges { target: incompleteBody; visible: true}
        },
        State {
            name: "collapsed"; when: root.collapsed
            PropertyChanges { target: attachments; visible: false}
            PropertyChanges { target: body; visible: false}
            PropertyChanges { target: footer; visible: false}
            PropertyChanges { target: collapsedBody; visible: true}
        }
    ]
    state: "full"

    Column {
        id: mainLayout
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Kube.Units.largeSpacing
        }
        height: childrenRect.height

        spacing: Kube.Units.smallSpacing

        //BEGIN header
        Item {
            id: header

            Layout.fillWidth: true
            anchors {
                left: parent.left
                right: parent.right
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
                    PropertyChanges { target: cc; visible: root.cc}
                    PropertyChanges { target: bcc; visible: root.bcc}
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

                font.pointSize: Kube.Units.tinyFontSize
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

                    Kube.SelectableLabel {
                        id: senderName

                        text: root.senderName
                        font.weight: Font.DemiBold
                        opacity: 0.75
                    }

                    Kube.SelectableLabel {
                        width: parent.width - senderName.width - date_label.width - Kube.Units.largeSpacing

                        text: root.sender
                        elide: Text.ElideRight
                        opacity: 0.75
                        clip: true

                        Kube.TextButton {
                            text: qsTr("Send mail to")
                            onClicked: Kube.Fabric.postMessage(Kube.Messages.compose, {"recipients": [root.sender]})
                        }
                    }
                }

                Kube.SelectableLabel {
                    id: subject

                    width: to.width

                    text: root.subject
                    elide: Text.ElideRight
                    opacity: 0.75
                    font.italic: true
                    states: [
                        State {
                            name: "trash"; when: root.trash
                            PropertyChanges { target: subject; text: qsTr("Trash: %1").arg(root.subject) }
                        },
                        State {
                            name: "draft"; when: root.draft
                            PropertyChanges { target: subject; text: qsTr("Draft: %1").arg(root.subject) }
                        },
                        State {
                            name: "sent"; when: root.sent
                            PropertyChanges { target: subject; text: qsTr("Sent: %1").arg(root.subject) }
                        }
                    ]
                }

                Kube.SelectableLabel {
                    id: recipients

                    width: parent.width - goDown.width - Kube.Units.smallSpacing

                    text:"to: "+ root.to + " "  + root.cc + " " +  root.bcc
                    elide: Text.ElideRight
                    opacity: 0.75
                }

                Kube.SelectableLabel {
                    id: to

                    width: parent.width - goDown.width - Kube.Units.smallSpacing

                    text:"to: " + root.to
                    wrapMode: Text.WordWrap
                    opacity: 0.75
                }

                Kube.SelectableLabel {
                    id: cc

                    width: parent.width - goDown.width - Kube.Units.smallSpacing

                    text:"cc: " + root.cc
                    wrapMode: Text.WordWrap
                    opacity: 0.75
                }

                Kube.SelectableLabel {
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

                //Only show the expand button if there is something to expand
                visible: recipients.truncated || root.cc || root.bcc

                height: Kube.Units.gridUnit
                width: height

                color: Kube.Colors.backgroundColor

                Kube.IconButton {
                    anchors.fill: parent
                    activeFocusOnTab: false

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
                left: parent.left
                right: parent.right
            }

            visible: !root.incomplete && !root.collapsed
            width: header.width - Kube.Units.largeSpacing
            height: visible ? implicitHeight : 0

            layoutDirection: Qt.RightToLeft
            spacing: Kube.Units.smallSpacing
            clip: true

            Repeater {
                model: messageParser.attachments

                delegate: AttachmentDelegate {
                    name: model.name
                    type: model.type
                    icon: model.iconName

                    clip: true

                    actionIcon: Kube.Icons.save_inverted
                    actionTooltip: qsTr("Save attachment")
                    onExecute: messageParser.attachments.saveAttachmentToDisk(messageParser.attachments.index(index, 0))
                    onClicked: messageParser.attachments.openAttachment(messageParser.attachments.index(index, 0))
                    onPublicKeyImport: messageParser.attachments.importPublicKey(messageParser.attachments.index(index, 0))
                }
            }
        }

        Item {
            id: body

            visible: true
            anchors {
                left: parent.left
                right: parent.right
            }
            height: visible ? mailViewer.height + 20 : 0

            MV.MailViewer {
                id: mailViewer
                anchors.top: body.top
                anchors.left: body.left
                anchors.right: body.right
                model: messageParser.parts
            }

        }

        Kube.Label {
            id: incompleteBody
            anchors {
                left: parent.left
                right: parent.right
            }
            visible: root.incomplete
            height: visible ? implicitHeight : 0
            text: qsTr("Incomplete body...")
            color: Kube.Colors.textColor
            enabled: false
            states: [
                State {
                    name: "inprogress"; when: model.status == Kube.MailListModel.InProgressStatus
                    PropertyChanges { target: incompleteBody; text: qsTr("Downloading message...") }
                },
                State {
                    name: "error"; when: model.status == Kube.MailListModel.ErrorStatus
                    PropertyChanges { target: incompleteBody; text: qsTr("Failed to download message...") }
                }
            ]
        }
        Kube.Button {
            id: collapsedBody
            anchors {
                left: parent.left
                right: parent.right
            }
            visible: false
            text: qsTr("Show")
            onClicked: root.collapsed = false
        }
        Item {
            id: footer
            property var mail: model.mail
            property string subject: model.subject

            anchors {
                left: parent.left
                right: parent.right
            }

            visible: true
            height: visible ? Kube.Units.gridUnit : 0
            width: parent.width

            Kube.TextButton {
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                }
                activeFocusOnTab: false

                text: model.trash ? qsTr("Delete Mail") : model.draft ? qsTr("Discard") : qsTr("Move to trash")
                opacity: 0.5
                onClicked: {
                    if (model.trash) {
                        Kube.Fabric.postMessage(Kube.Messages.remove, {"mail": model.mail})
                    } else {
                        Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": model.mail})
                    }
                }
            }

            Row {
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }
                spacing: Kube.Units.smallSpacing

                Kube.Button {
                    visible: !model.trash && !model.draft
                    activeFocusOnTab: false

                    text: qsTr("Share")
                    onClicked: {
                        Kube.Fabric.postMessage(Kube.Messages.forward, {"mail": model.mail})
                    }
                }

                Kube.Button {
                    visible: !model.trash
                    activeFocusOnTab: false

                    text: model.draft ? qsTr("Edit") : qsTr("Reply")
                    onClicked: {
                        if (model.draft) {
                            Kube.Fabric.postMessage(Kube.Messages.edit, {"mail": model.mail})
                        } else {
                            Kube.Fabric.postMessage(Kube.Messages.reply, {"mail": model.mail})
                        }
                    }
                }
                Row {
                    Kube.ExtensionPoint {
                        extensionPoint: "extensions/mailview"
                        context: {"mail": footer.mail, "subject": footer.subject, "accountId": currentAccount}
                    }
                }
            }
        }
    } //ColumnLayout

    //Dimm unread messages
    Rectangle {
        anchors.fill: parent
        color: Kube.Colors.buttonColor
        opacity: 0.4
        visible: root.unread
    }

    Component {
        id: debugPopupComponent
        Kube.Popup {
            id: debugPopup
            modal: true
            parent: ApplicationWindow.overlay
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
            x: (parent.width - width)/2
            y: Kube.Units.largeSpacing
            width: parent.width / 2
            height: parent.height - Kube.Units.largeSpacing * 2
            clip: true

            Flickable {
                id: flickable
                anchors.fill: parent
                ScrollBar.vertical: Kube.ScrollBar {}
                contentHeight: content.height
                contentWidth: parent.width
                Column {
                    id: content
                    width: flickable.width
                    height: childrenRect.height

                    TextEdit {
                        id: structure
                        width: parent.width
                        readOnly: true
                        selectByMouse: true
                        textFormat: TextEdit.PlainText
                        wrapMode: TextEdit.Wrap
                        height: implicitHeight
                        text: messageParser.structureAsString
                    }

                    TextEdit {
                        id: rawContent
                        width: parent.width
                        readOnly: true
                        selectByMouse: true
                        textFormat: TextEdit.PlainText
                        wrapMode: TextEdit.Wrap
                        height: implicitHeight
                        text: messageParser.rawContent.substring(0, 100000) //The TextEdit deals poorly with messages that are too large.
                    }
                    Rectangle {
                        color: "black"
                        height: 2
                    }
                    Controls1.TreeView {
                        id: mailStructure
                        width: parent.width
                        height: implicitHeight
                        Controls1.TableViewColumn {
                            role: "type"
                            title: "Type"
                        }
                        Controls1.TableViewColumn {
                            role: "embedded"
                            title: "Embedded"
                        }
                        Controls1.TableViewColumn {
                            role: "securityLevel"
                            title: "SecurityLevel"
                        }
                        Controls1.TableViewColumn {
                            role: "content"
                            title: "Content"
                        }
                        model: messageParser.parts
                        itemDelegate: Item {
                            property variant currentData: styleData.value
                            Text {
                                anchors.fill: parent
                                color: styleData.textColor
                                elide: Text.ElideRight
                                text: styleData.value ? styleData.value : ""
                                textFormat: Text.PlainText
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
                }
            }
        }
    }
}
