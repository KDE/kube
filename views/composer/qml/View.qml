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
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0 as Dialogs

import org.kube.framework 1.0 as Kube

Kube.View {
    id: root

    visibleViews: 2
    property bool newMessage: false
    property int loadType: Kube.ComposerController.Draft
    property variant message: {}
    property variant recipients: []
    property variant accountId: {}

    resources: [
        Kube.ComposerController {
            id: composerController
            objectName: "composerController"
            sign: signCheckbox.enabled && signCheckbox.checked
            encrypt: encryptCheckbox.enabled && encryptCheckbox.checked
            onDone: root.done()

            property bool foundAllKeys: composerController.to.foundAllKeys && composerController.cc.foundAllKeys && composerController.bcc.foundAllKeys

            sendAction.enabled: composerController.accountId &&
                                composerController.subject &&
                                (!composerController.encrypt || composerController.foundAllKeys) &&
                                (!composerController.sign && !composerController.encrypt || composerController.foundPersonalKeys) &&
                                !composerController.to.empty
            saveAsDraftAction.enabled: composerController.accountId
            onMessageLoaded: { editorPage.initialText = body }
            onCleared: { editorPage.initialText = "" }
        }
    ]

    onSetup: {
        loadMessage(root.message, root.loadType)
        if (root.accountId) {
            composerController.identitySelector.currentAccountId = root.accountId
        }
    }

    onRefresh: {
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "mail", "specialPurpose": "drafts"})
        //For autocompletion
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "contacts"})
    }

    onAborted: {
        //Avoid loosing the message
        if (composerController.saveAsDraftAction.enabled) {
            composerController.saveAsDraftAction.execute()
        }
    }

    function loadMessage(message, loadType) {
        if (message) {
            switch(loadType) {
                case Kube.ComposerController.Draft:
                    composerController.loadDraft(message)
                    break;
                case Kube.ComposerController.Reply:
                    composerController.loadReply(message)
                    editorPage.focusBody()
                    break;
                case Kube.ComposerController.Forward:
                    composerController.loadForward(message)
                    editorPage.forceActiveFocus()
                    break;
            }

        } else if (newMessage) {
            composerController.clear()
            if (root.recipients) {
                for (var i = 0; i < root.recipients.length; ++i) {
                    composerController.to.add({name: root.recipients[i]})
                }
            }
            editorPage.forceActiveFocus()
        }
    }

    function closeFirstSplitIfNecessary() {
        //Move the view forward
        if (root.currentIndex == 0) {
            root.incrementCurrentIndex()
        }
    }

    //Drafts
    Rectangle {
        width: Kube.Units.gridUnit * 15
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        Layout.fillHeight: true

        color: Kube.Colors.darkBackgroundColor

        ColumnLayout {

            anchors {
                fill: parent
                topMargin: Kube.Units.largeSpacing
                leftMargin: Kube.Units.largeSpacing
            }

            spacing: Kube.Units.largeSpacing

            Kube.PositiveButton {
                objectName: "newMailButton"

                width: parent.width - Kube.Units.largeSpacing
                focus: true
                text: qsTr("New Email")
                onClicked: {
                    listView.currentIndex = -1
                    composerController.clear()
                    editorPage.forceActiveFocus()
                }
            }

            Kube.Label{
                text: qsTr("Drafts")
                color: Kube.Colors.highlightedTextColor
                font.weight: Font.Bold
            }

            Kube.ListView {
                id: listView
                activeFocusOnTab: true

                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                currentIndex: -1
                highlightFollowsCurrentItem: false

                //BEGIN keyboard nav
                onActiveFocusChanged: {
                    if (activeFocus && currentIndex < 0) {
                        currentIndex = 0
                    }
                }
                //END keyboard nav

                onCurrentItemChanged: {
                    if (currentItem) {
                        root.loadMessage(currentItem.currentData.domainObject, Kube.ComposerController.Draft)
                    }
                }

                model: Kube.MailListModel {
                    id: mailListModel
                    filter: {
                        "drafts": true,
                        "headersOnly": false,
                        "fetchMails": true
                    }
                }

                delegate: Kube.ListDelegate {
                    id: delegateRoot

                    color: Kube.Colors.darkBackgroundColor
                    border.width: 0

                    Item {
                        id: content

                        anchors {
                            fill: parent
                            margins: Kube.Units.smallSpacing
                        }

                        Kube.Label {
                            width: content.width - Kube.Units.largeSpacing
                            text: model.subject == "" ? "no subject" : model.subject
                            color: Kube.Colors.highlightedTextColor
                            maximumLineCount: 2
                            wrapMode: Text.WrapAnywhere
                            elide: Text.ElideRight
                        }

                        Kube.Label {
                            anchors {
                                right: parent.right
                                rightMargin: Kube.Units.largeSpacing
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
                            bottomMargin: Kube.Units.smallSpacing
                            rightMargin: Kube.Units.largeSpacing
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
    EditorPage {
        id: editorPage
        Layout.fillWidth: true
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        Layout.fillHeight: true

        controller: composerController
        onDone: recipients.forceActiveFocus(Qt.TabFocusReason)
        onFocusChange: closeFirstSplitIfNecessary()
    }

    //Recipients
    FocusScope {
        id: recipients
        Layout.fillHeight: true
        width: Kube.Units.gridUnit * 15
        activeFocusOnTab: true

        //background
        Rectangle {
            anchors.fill: parent
            color: Kube.Colors.backgroundColor

            Rectangle {
                height: parent.height
                width: 1
                color: Kube.Colors.buttonColor
            }
        }

        //Content
        ColumnLayout {
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
            }

            spacing: Kube.Units.largeSpacing
            ColumnLayout {
                Layout.maximumWidth: parent.width
                Layout.fillWidth: true
                Layout.fillHeight: true

                AddresseeListEditor {
                    label: qsTr("Sending email to:")
                    Layout.preferredHeight: implicitHeight
                    Layout.fillWidth: true
                    focus: true
                    activeFocusOnTab: true
                    encrypt: composerController.encrypt
                    controller: composerController.to
                    completer: composerController.recipientCompleter
                }

                AddresseeListEditor {
                    label: qsTr("Sending copy to (Cc):")
                    Layout.preferredHeight: implicitHeight
                    Layout.fillWidth: true
                    activeFocusOnTab: true
                    encrypt: composerController.encrypt
                    controller: composerController.cc
                    completer: composerController.recipientCompleter
                }

                AddresseeListEditor {
                    label: qsTr("Sending secret copy to (Bcc):")
                    Layout.preferredHeight: implicitHeight
                    Layout.fillWidth: true
                    activeFocusOnTab: true
                    encrypt: composerController.encrypt
                    controller: composerController.bcc
                    completer: composerController.recipientCompleter
                }
                Item {
                    width: parent.width
                    Layout.fillHeight: true
                }
            }

            RowLayout {
                enabled: composerController.foundPersonalKeys
                Kube.CheckBox {
                    id: encryptCheckbox
                    checked: composerController.encrypt
                }
                Kube.Label {
                    text: qsTr("encrypt")
                }
            }

            RowLayout {
                enabled: composerController.foundPersonalKeys
                Kube.CheckBox {
                    id: signCheckbox
                    checked: composerController.sign
                }
                Kube.Label {
                    text: qsTr("sign")
                }
            }
            Kube.Label {
                visible: !composerController.foundPersonalKeys
                Layout.maximumWidth: parent.width
                text: qsTr("Encryption is not available because your personal key has not been found.")
                wrapMode: Text.Wrap
            }

            RowLayout {
                Layout.maximumWidth: parent.width
                width: parent.width
                height: Kube.Units.gridUnit

                Kube.Button {
                    width: saveDraftButton.width
                    text: qsTr("Discard")
                    onClicked: root.done()
                }

                Kube.Button {
                    id: saveDraftButton

                    text: qsTr("Save as Draft")
                    enabled: composerController.saveAsDraftAction.enabled
                    onClicked: {
                        composerController.saveAsDraftAction.execute()
                    }
                }
            }

            ColumnLayout {
                Layout.maximumWidth: parent.width
                Layout.fillWidth: true
                Kube.Label {
                    id: fromLabel
                    text: qsTr("You are sending this from:")
                }

                Kube.ComboBox {
                    id: identityCombo
                    objectName: "identityCombo"

                    width: parent.width - Kube.Units.largeSpacing * 2

                    model: composerController.identitySelector.model
                    textRole: "address"
                    Layout.fillWidth: true
                    //A regular binding is not enough in this case, we have to use the Binding element
                    Binding { target: identityCombo; property: "currentIndex"; value: composerController.identitySelector.currentIndex }
                    onCurrentIndexChanged: {
                        composerController.identitySelector.currentIndex = currentIndex
                    }
                }
            }

            Kube.PositiveButton {
                objectName: "sendButton"
                id: sendButton

                width: parent.width

                text: qsTr("Send")
                enabled: composerController.sendAction.enabled
                onClicked: {
                    composerController.sendAction.execute()
                }
            }
        }
    }//FocusScope
}
