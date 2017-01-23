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

import QtQml 2.2 as QtQml

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.actions 1.0 as KubeAction

Rectangle {
    id: root

    property variant mail;

    color: Kirigami.Theme.backgroundColor

    ListView {
        id: listView
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        clip: true

        model: KubeFramework.MailListModel {
            mail: root.mail
        }

        header: Item {
            height: Kirigami.Units.gridUnit * 0.5
            width: parent.width

        }

        footer: Item {
            height: Kirigami.Units.gridUnit
            width: parent.width
        }

        delegate: mailDelegate

        //Intercept all scroll events,
        //necessary due to the webengineview
        KubeFramework.MouseProxy {
            anchors.fill: parent
            target: listView
            forwardWheelEvents: true
        }
    }
    Component {
        id: mailDelegate

        Item {

            height: sheet.height + Kirigami.Units.gridUnit
            width: parent.width

            Rectangle {
                id: sheet
                anchors.centerIn: parent
                implicitHeight: header.height + attachments.height + body.height + footer.height + Kirigami.Units.largeSpacing
                width: parent.width - Kirigami.Units.gridUnit * 2

                color: Kirigami.Theme.viewBackgroundColor

                //BEGIN header
                Item {
                    id: header

                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        margins: Kirigami.Units.largeSpacing
                    }

                    height: headerContent.height + Kirigami.Units.smallSpacing

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

                    Text {
                        id: date_label

                        anchors {
                            right: seperator.right
                            top: parent.top
                        }

                        text: Qt.formatDateTime(model.date, "dd MMM yyyy hh:mm")

                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.7
                        color: Kirigami.Theme.textColor
                        opacity: 0.75
                    }

                    Column {
                        id: headerContent

                        anchors {
                            //left: to_l.right
                            horizontalCenter: parent.horizontalCenter
                        }

                        //spacing: Kirigami.Units.smallSpacing

                        width: parent.width

                        Row{
                            id: from

                            width: parent.width

                            spacing: Kirigami.Units.smallSpacing
                            clip: true

                            Text {
                                id: senderName

                                text: model.senderName

                                font.weight: Font.DemiBold
                                color: Kirigami.Theme.textColor
                                opacity: 0.75
                            }

                            Text {

                                text: model.sender

                                width: parent.width - senderName.width - date_label.width - Kirigami.Units.largeSpacing
                                elide: Text.ElideRight

                                color: Kirigami.Theme.textColor
                                opacity: 0.75

                                clip: true
                            }
                        }

                        Text {
                            id: subject

                            width: to.width

                            text: model.subject

                            elide: Text.ElideRight

                            color: Kirigami.Theme.textColor
                            opacity: 0.75
                            font.italic: true
                        }

                        Text {
                            id: recipients

                            width: parent.width - goDown.width - Kirigami.Units.smallSpacing

                            text:"to: "+ model.to + " "  + model.cc + " " +  model.bcc

                            elide: Text.ElideRight

                            color: Kirigami.Theme.textColor
                            opacity: 0.75
                        }

                        Text {
                            id: to

                            width: parent.width - goDown.width - Kirigami.Units.smallSpacing

                            text:"to: " + model.to

                            wrapMode: Text.WordWrap
                            color: Kirigami.Theme.textColor
                            opacity: 0.75
                        }

                        Text {
                            id: cc

                            width: parent.width - goDown.width - Kirigami.Units.smallSpacing

                            text:"cc: " + model.cc

                            wrapMode: Text.WordWrap
                            color: Kirigami.Theme.textColor
                            opacity: 0.75
                        }

                        Text {
                            id: bcc

                            width: parent.width - goDown.width - Kirigami.Units.smallSpacing

                            text:"bcc: " + model.bcc

                            wrapMode: Text.WordWrap
                            color: Kirigami.Theme.textColor
                            opacity: 0.75
                        }

                    }
                    Rectangle {
                        id: goDown
                        anchors {
                            bottom: seperator.top
                            right: seperator.right
                        }

                        height: Kirigami.Units.gridUnit
                        width: height

                        color: Kirigami.Theme.backgroundColor

                        Controls1.ToolButton {
                            anchors.fill: parent

                            iconName: "go-down"
                        }
                    }

                    Rectangle {
                        anchors {
                            bottom: seperator.top
                            right: seperator.right
                        }

                        height: Kirigami.Units.gridUnit
                        width: height

                        color: Kirigami.Theme.backgroundColor

                        Controls1.ToolButton {
                            anchors.fill: parent

                            iconName: header.state === "details" ? "go-up" : "go-down"

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

                        color: Kirigami.Theme.textColor
                        opacity: 0.5
                    }
                }
                //END header

                Flow {
                    id: attachments

                    anchors {
                        top: header.bottom
                        topMargin: Kirigami.Units.smallSpacing
                        right: header.right
                    }

                    width: header.width - Kirigami.Units.largeSpacing

                    layoutDirection: Qt.RightToLeft
                    spacing: Kirigami.Units.smallSpacing
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
                        leftMargin: Kirigami.Units.largeSpacing
                        rightMargin: Kirigami.Units.largeSpacing
                        topMargin: Math.max(attachments.height, Kirigami.Units.largeSpacing)
                    }

                    width: header.width - Kirigami.Units.largeSpacing * 2
                    height: desiredHeight

                    message: model.mimeMessage
                }

                Item {
                    id: footer

                    anchors.bottom: parent.bottom

                    height: Kirigami.Units.gridUnit * 2
                    width: parent.width

                    Text {
                        anchors{
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kirigami.Units.largeSpacing
                        }

                        KubeFramework.MailController {
                            id: mailController
                            mail: model.mail
                        }

                        text: model.trash ? qsTr("Delete Mail") : qsTr("Move to trash")
                        color: Kirigami.Theme.textColor
                        opacity: 0.5
                        enabled: model.trash ? mailController.removeAction.enabled : mailController.moveToTrashAction.enabled
                        MouseArea {
                            anchors.fill: parent
                            enabled: parent.enabled
                            onClicked: {
                                if (model.trash) {
                                    mailController.removeAction.execute();
                                } else {
                                    mailController.moveToTrashAction.execute();
                                }
                            }
                        }
                    }

                    Controls1.ToolButton {
                        visible: !model.trash
                        anchors{
                            verticalCenter: parent.verticalCenter
                            right: parent.right
                            rightMargin: Kirigami.Units.largeSpacing
                        }

                        KubeAction.Context {
                            id: mailcontext
                            property variant mail
                            property bool isDraft
                            mail: model.mail
                            isDraft: model.draft
                        }

                        KubeAction.Action {
                            id: replyAction
                            actionId: "org.kde.kube.actions.reply"
                            context: maillistcontext
                        }

                        KubeAction.Action {
                            id: editAction
                            actionId: "org.kde.kube.actions.edit"
                            context: maillistcontext
                        }

                        iconName: model.draft ? "document-edit" : "mail-reply-sender"
                        onClicked: {
                            if (model.draft) {
                                editAction.execute()
                            } else {
                                replyAction.execute()
                            }
                        }
                    }
                }
            }
        }
    }
}
