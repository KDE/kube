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
import QtQuick.Controls 2.0 as Controls2
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

    resources: [
        Kube.ComposerController {
            id: composerController
            sign: signCheckbox.checked
            encrypt: encryptCheckbox.checked
            onDone: root.done()

            property bool foundAllKeys: composerController.to.foundAllKeys && composerController.cc.foundAllKeys && composerController.bcc.foundAllKeys

            sendAction.enabled: composerController.accountId && composerController.subject && (!composerController.encrypt || composerController.foundAllKeys) && (!composerController.sign && !composerController.encrypt || composerController.foundPersonalKeys) && !composerController.to.empty
            saveAsDraftAction.enabled: composerController.accountId
            onMessageLoaded: { textEditor.initialText = body }
            onCleared: { textEditor.initialText = "" }
        }
    ]

    onSetup: {
        loadMessage(root.message, root.loadType)
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
                    subject.forceActiveFocus()
                    break;
                case Kube.ComposerController.Forward:
                    composerController.loadForward(message)
                    subject.forceActiveFocus()
                    break;
            }

        } else if (newMessage) {
            composerController.clear()
            if (root.recipients) {
                for (var i = 0; i < root.recipients.length; ++i) {
                    composerController.to.add({name: root.recipients[i]})
                }
            }
            subject.forceActiveFocus()
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

        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        width: Kube.Units.gridUnit * 15
        Layout.minimumWidth: Kube.Units.gridUnit * 5

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
                    subject.forceActiveFocus()
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

                anchors {
                    left: parent.left
                    right: parent.right
                }

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
                Keys.onDownPressed: {
                    listView.incrementCurrentIndex()
                }
                Keys.onUpPressed: {
                    listView.decrementCurrentIndex()
                }
                //END keyboard nav

                onCurrentItemChanged: {
                    if (currentItem) {
                        root.loadMessage(currentItem.currentData.domainObject, Kube.ComposerController.Draft)
                    }
                }

                model: Kube.MailListModel {
                    id: mailListModel
                    showDrafts: true
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

            spacing: Kube.Units.smallSpacing

            Kube.TextField {
                id: subject
                objectName: "subject"
                Layout.fillWidth: true
                activeFocusOnTab: true
                font.bold: true

                placeholderText: qsTr("Enter Subject...")
                text: composerController.subject
                onTextChanged: composerController.subject = text;
                onActiveFocusChanged: {
                    if (activeFocus) {
                        closeFirstSplitIfNecessary()
                    }
                }
            }

            Flow {
                id: attachments

                Layout.fillWidth: true
                layoutDirection: Qt.RightToLeft
                spacing: Kube.Units.smallSpacing
                clip: true

                Repeater {
                    model: composerController.attachments.model
                    delegate: Kube.AttachmentDelegate {
                        name: model.filename ? model.filename : ""
                        icon: model.iconname ? model.iconname : ""
                        clip: true
                        actionIcon: Kube.Icons.remove
                        onExecute: composerController.attachments.remove(model.id)
                    }
                }
            }

            RowLayout {

                spacing: Kube.Units.largeSpacing

                Row {
                    spacing: 1

                    Kube.IconButton {
                        iconName: Kube.Icons.bold
                        checkable: true
                        checked: textEditor.bold
                        onClicked: textEditor.bold = !textEditor.bold
                        focusPolicy: Qt.TabFocus
                        focus: false
                    }
                    Kube.IconButton {
                        iconName: Kube.Icons.italic
                        checkable: true
                        checked: textEditor.italic
                        onClicked: textEditor.italic = !textEditor.italic
                        focusPolicy: Qt.TabFocus
                        focus: false
                    }
                    Kube.IconButton {
                        iconName: Kube.Icons.underline
                        checkable: true
                        checked: textEditor.underline
                        onClicked: textEditor.underline = !textEditor.underline
                        focusPolicy: Qt.TabFocus
                        focus: false
                    }
                    Kube.TextButton {
                        id: deleteButton
                        text: qsTr("Remove Formatting")
                        visible: textEditor.htmlEnabled
                        onClicked: textEditor.clearFormatting()
                    }
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
                            title: qsTr("Choose a file to attach")
                            folder: shortcuts.home
                            selectFolder: false
                            selectExisting: true
                            selectMultiple: true
                            onAccepted: {
                                for (var i = 0; i < fileDialog.fileUrls.length; ++i) {
                                    composerController.attachments.add({url: fileDialog.fileUrls[i]})
                                }
                            }
                        }
                    }
                }
            }

            Kube.TextEditor {
                id: textEditor
                objectName: "textEditor"
                activeFocusOnTab: true

                Layout.fillWidth: true
                Layout.fillHeight: true
                onHtmlEnabledChanged: {
                    composerController.htmlBody = htmlEnabled;
                }

                onActiveFocusChanged: closeFirstSplitIfNecessary()
                Keys.onEscapePressed: recipients.forceActiveFocus(Qt.TabFocusReason)
                onTextChanged: {
                    composerController.body = text;
                }
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

                Kube.Label {
                    text: qsTr("Sending Email to:")
                }

                AddresseeListEditor {
                    Layout.preferredHeight: implicitHeight
                    Layout.fillWidth: true
                    focus: true
                    activeFocusOnTab: true
                    encrypt: composerController.encrypt
                    controller: composerController.to
                    completer: composerController.recipientCompleter
                }

                Kube.Label {
                    text: qsTr("Sending Copy to (CC):")
                }
                AddresseeListEditor {
                    id: cc
                    Layout.preferredHeight: cc.implicitHeight
                    Layout.fillWidth: true
                    activeFocusOnTab: true
                    encrypt: composerController.encrypt
                    controller: composerController.cc
                    completer: composerController.recipientCompleter
                }

                Kube.Label {
                    text: qsTr("Sending Secret Copy to (Bcc):")
                }
                AddresseeListEditor {
                    id: bcc
                    Layout.preferredHeight: bcc.implicitHeight
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
                    checked: false
                }
                Kube.Label {
                    text: qsTr("encrypt")
                }
            }

            RowLayout {
                enabled: composerController.foundPersonalKeys
                Kube.CheckBox {
                    id: signCheckbox
                    checked: false
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

                    width: parent.width - Kube.Units.largeSpacing * 2

                    model: composerController.identitySelector.model
                    textRole: "address"
                    Layout.fillWidth: true
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
