/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
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


import QtQuick 2.7
import QtQuick.Controls 1.3
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0 as Dialogs

import org.kube.framework 1.0 as Kube

Kube.View {
    id: root

    property bool newMessage: false
    property bool loadAsDraft: false
    property variant message: {}
    property variant recipients: []

    //FIXME mean hack to unfuck hiding
    property variant _composerController: Kube.ComposerController {
        id: composerController
        onDone: Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
    }

    //actions
    property variant sendAction: composerController.sendAction
    property variant saveAsDraftAction: composerController.saveAsDraftAction

    Component.onCompleted: loadMessage(root.message, root.loadAsDraft)

    Controls2.StackView.onActivated: {
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "mail", "specialPurpose": "drafts"})
        //For autocompletion
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "contacts"})
    }

    function loadMessage(message, loadAsDraft) {
        if (message) {
            composerController.loadMessage(message, loadAsDraft)
            //Forward focus for replies directly
            if (!loadAsDraft) {
                subject.forceActiveFocus()
            }
        } else if (newMessage) {
            composerController.clear()
            if (root.recipients) {
                for (var i = 0; i < root.recipients.length; ++i) {
                    composerController.addTo(root.recipients[i])
                }
            }
            subject.forceActiveFocus()
        }
    }

    function closeFirstSplitIfNecessary() {
        //Move the view forward
        if (root.currentIndex == 0) {
            listView.currentIndex = -1
            root.incrementCurrentIndex()
        }
    }

    //Drafts
    Rectangle {

        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        width: Kube.Units.gridUnit * 15
        Layout.minimumWidth: Kube.Units.gridUnit * 5

        color: Kube.Colors.textColor

        ColumnLayout {

            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
            }

            spacing: Kube.Units.largeSpacing

            Kube.PositiveButton {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                focus: true
                text: qsTr("New Email")
                onClicked: {
                    composerController.clear()
                    subject.forceActiveFocus()
                }
            }

            Kube.Label{
                text: qsTr("Drafts")
                color: Kube.Colors.highlightedTextColor
            }

            Kube.ListView {
                id: listView
                activeFocusOnTab: true

                anchors {
                    left: parent.left
                    right: parent.right
                }

                Layout.fillHeight: true
                clip: true

                //BEGIN keyboard nav
                onActiveFocusChanged: {
                    if (activeFocus && currentIndex < 0) {
                        currentIndex = 0
                    }
                }
                Keys.onDownPressed: {
                    listView.incrementCurrentIndex()
                }
                Keys.onUpPressed: {
                    listView.decrementCurrentIndex()
                }
                //END keyboard nav

                onCurrentItemChanged: {
                    if (currentItem) {
                        root.loadMessage(currentItem.currentData.domainObject, true)
                    }
                }

                model: Kube.MailListModel {
                    id: mailListModel
                    showDrafts: true
                }

                delegate: Kube.ListDelegate {
                    id: delegateRoot

                    color: Kube.Colors.textColor
                    border.width: 0

                    Item {
                        id: content

                        anchors {
                            fill: parent
                            margins: Kube.Units.smallSpacing
                        }

                        Kube.Label{
                            width: content.width - Kube.Units.largeSpacing * 2
                            text: model.subject
                            color: Kube.Colors.highlightedTextColor
                            maximumLineCount: 2
                            wrapMode: Text.WrapAnywhere
                            elide: Text.ElideRight
                        }

                        Kube.Label {
                            anchors {
                                right: parent.right
                                bottom: parent.bottom
                            }
                            text: Qt.formatDateTime(model.date, "dd MMM yyyy")
                            font.italic: true
                            color: Kube.Colors.disabledTextColor
                            font.pointSize: Kube.Units.smallFontSize
                            visible: !delegateRoot.hovered
                        }
                    }
                    Row {
                        id: buttons

                        anchors {
                            right: parent.right
                            bottom: parent.bottom
                            margins: Kube.Units.smallSpacing
                        }

                        visible: delegateRoot.hovered
                        spacing: Kube.Units.smallSpacing
                        opacity: 0.7

                        Kube.IconButton {
                            id: deleteButton
                            activeFocusOnTab: true
                            iconName: Kube.Icons.moveToTrash
                            visible: enabled
                            enabled: !!model.mail
                            onClicked: Kube.Fabric.postMessage(Kube.Messages.moveToTrash, {"mail": model.mail})
                        }
                    }
                }
            }
        }
    }

    //Content
    Rectangle {
        Layout.fillWidth: true
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        anchors {
            top: parent.top
            bottom: parent.bottom
        }
        color: Kube.Colors.backgroundColor

        ColumnLayout {
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
                leftMargin: Kube.Units.largeSpacing + Kube.Units.gridUnit * 2
                rightMargin: Kube.Units.largeSpacing + Kube.Units.gridUnit * 2
            }

            spacing: Kube.Units.smallSpacing * 2

            Kube.TextField {
                id: subject
                Layout.fillWidth: true
                activeFocusOnTab: true

                placeholderText: qsTr("Enter Subject...")
                text: composerController.subject
                onTextChanged: composerController.subject = text;
                onActiveFocusChanged: closeFirstSplitIfNecessary()
            }

            Flow {
                id: attachments

                Layout.fillWidth: true
                layoutDirection: Qt.RightToLeft
                spacing: Kube.Units.smallSpacing
                clip: true

                Repeater {
                    model: composerController.attachmentModel
                    delegate: Kube.AttachmentDelegate {
                        name: model.filename
                        icon: model.iconName
                        clip: true
                        actionIcon: Kube.Icons.remove
                        onExecute: composerController.removeAttachment(model.url)
                    }
                }
            }

            RowLayout {
                Kube.Button {
                    text: "b"
                    checkable: true
                }
                Kube.Button {
                    text: "i"
                    checkable: true
                }
                Kube.Button {
                    text: "u"
                    checkable: true
                }

                Item {
                    height: 1
                    Layout.fillWidth: true
                }

                Kube.Button {
                    text: qsTr("Attach file")

                    onClicked: {
                        fileDialogComponent.createObject(parent)
                    }

                    Component {
                        id: fileDialogComponent
                        Dialogs.FileDialog {
                            id: fileDialog
                            visible: true
                            title: "Choose a file to attach"
                            selectFolder: false
                            onAccepted: {
                                composerController.addAttachment(fileDialog.fileUrl)
                            }
                        }
                    }
                }
            }

            Kube.TextEditor {
                Layout.fillWidth: true
                Layout.fillHeight: true
                activeFocusOnTab: true
                onActiveFocusChanged: closeFirstSplitIfNecessary()
                text: composerController.body
                onTextChanged: composerController.body = text;
                Keys.onEscapePressed: recipients.forceActiveFocus()
            }
        }
    }

    //Recepients
    FocusScope {
        id: recipients
        anchors {
            top: parent.top
            bottom: parent.bottom
        }
        activeFocusOnTab: true
        width: Kube.Units.gridUnit * 15
        Rectangle {
            anchors.fill: parent
            color: Kube.Colors.backgroundColor

            Rectangle {
                height: parent.height
                width: 1
                color: Kube.Colors.buttonColor
            }

            //Content
            Item {
                anchors.right: parent.right
                width: parent.width
                height: parent.height

                ColumnLayout {
                    anchors {
                        top: parent.top
                        bottom: bottomButtons.top
                        left: parent.left
                        right: parent.right
                        margins: Kube.Units.largeSpacing
                    }

                    Kube.Label {
                        text: qsTr("Sending Email to:")
                    }

                    AddresseeListEditor {
                        id: to
                        Layout.preferredHeight: to.implicitHeight
                        Layout.fillWidth: true
                        focus: true
                        activeFocusOnTab: true
                        completer: composerController.recipientCompleter
                        model: composerController.toModel
                        onAdded: composerController.addTo(text)
                        onRemoved: composerController.removeTo(text)
                    }

                    Kube.Label {
                        text: qsTr("Sending Copy to (CC):")
                    }
                    AddresseeListEditor {
                        id: cc
                        Layout.preferredHeight: cc.implicitHeight
                        Layout.fillWidth: true
                        activeFocusOnTab: true
                        completer: composerController.recipientCompleter
                        model: composerController.ccModel
                        onAdded: composerController.addCc(text)
                        onRemoved: composerController.removeCc(text)
                    }

                    Kube.Label {
                        text: qsTr("Sending Secret Copy to (Bcc):")
                    }
                    AddresseeListEditor {
                        id: bcc
                        Layout.preferredHeight: bcc.implicitHeight
                        Layout.fillWidth: true
                        activeFocusOnTab: true
                        completer: composerController.recipientCompleter
                        model: composerController.bccModel
                        onAdded: composerController.addBcc(text)
                        onRemoved: composerController.removeBcc(text)
                    }

                    Item {
                        width: parent.width
                        Layout.fillHeight: true
                    }
                }


                Item {
                    id: bottomButtons
                    anchors {
                        bottom: fromLabel.top
                        bottomMargin: Kube.Units.largeSpacing
                        horizontalCenter: parent.horizontalCenter
                    }
                    width: parent.width - Kube.Units.largeSpacing * 2
                    height: Kube.Units.gridUnit

                    Kube.Button {
                        id: saveDraftButton

                        anchors.right: parent.right

                        text: qsTr("Save as Draft")
                        //TODO enabled: saveAsDraftAction.enabled
                        onClicked: {
                            saveAsDraftAction.execute()
                        }
                    }
                    Kube.Button {
                        anchors.left: parent.left

                        text: qsTr("Discard")
                        onClicked: Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
                    }
                }

                Kube.Label {
                    id: fromLabel
                    anchors {
                        bottom: identityCombo.top
                        bottomMargin: Kube.Units.smallSpacing
                        left: identityCombo.left
                    }
                    text: qsTr("You are sending this from:")
                }

                Kube.ComboBox {
                    id: identityCombo

                    anchors {
                        bottom: sendButton.top
                        bottomMargin: Kube.Units.largeSpacing
                        horizontalCenter: parent.horizontalCenter
                    }
                    width: parent.width - Kube.Units.largeSpacing * 2

                    model: composerController.identitySelector.model
                    textRole: "address"
                    Layout.fillWidth: true
                    onCurrentIndexChanged: {
                        composerController.identitySelector.currentIndex = currentIndex
                    }
                }

                Kube.PositiveButton {
                    id: sendButton

                    anchors {
                        bottom: parent.bottom
                        bottomMargin: Kube.Units.largeSpacing
                        horizontalCenter: parent.horizontalCenter
                    }
                    width: parent.width - Kube.Units.largeSpacing * 2

                    text: qsTr("Send")
                    enabled: sendAction.enabled
                    onClicked: {
                        sendAction.execute()
                    }
                }
            }
        }
    } //FocusScope
}
