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

FocusScope {
    id: root

    property alias initialText: textEditor.initialText
    property var controller

    signal done()
    signal focusChange()

    Rectangle {
        anchors.fill: parent
        color: Kube.Colors.paperWhite

        ColumnLayout {
            anchors {
                top: parent.top
                bottom: parent.bottom
                margins: Kube.Units.largeSpacing
                horizontalCenter: parent.horizontalCenter
            }
            property var preferredWidth: Kube.Units.gridUnit * 24
            property var minimumWidth: root.width - (Kube.Units.largeSpacing + Kube.Units.gridUnit * 2) * 2
            width: Math.min(minimumWidth, preferredWidth)

            spacing: Kube.Units.smallSpacing

            Flow {
                id: attachments

                Layout.fillWidth: true
                layoutDirection: Qt.RightToLeft
                spacing: Kube.Units.smallSpacing
                clip: true

                Repeater {
                    model: controller.attachments.model
                    delegate: Kube.AttachmentDelegate {
                        name: model.filename ? model.filename : ""
                        icon: model.iconname ? model.iconname : ""
                        clip: true
                        actionIcon: Kube.Icons.remove
                        onExecute: controller.attachments.remove(model.id)
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
                        opacity: activeFocus || hovered ? 1 : 0.6
                    }
                    Kube.IconButton {
                        iconName: Kube.Icons.italic
                        checkable: true
                        checked: textEditor.italic
                        onClicked: textEditor.italic = !textEditor.italic
                        focusPolicy: Qt.TabFocus
                        focus: false
                        opacity: activeFocus || hovered ? 1 : 0.6
                    }
                    Kube.IconButton {
                        iconName: Kube.Icons.underline
                        checkable: true
                        checked: textEditor.underline
                        onClicked: textEditor.underline = !textEditor.underline
                        focusPolicy: Qt.TabFocus
                        focus: false
                        opacity: activeFocus || hovered ? 1 : 0.6
                    }
                    Kube.TextButton {
                        id: deleteButton
                        text: qsTr("Remove Formatting")
                        visible: textEditor.htmlEnabled
                        onClicked: textEditor.clearFormatting()
                        opacity: activeFocus || hovered ? 1 : 0.6
                    }
                }

                Item {
                    height: 1
                    Layout.fillWidth: true
                }

                Kube.Button {
                    text: qsTr("Attach file")

                    onClicked: {
                        fileDialog.open()
                    }
                    opacity: activeFocus || hovered ? 1 : 0.6

                    Dialogs.FileDialog {
                        id: fileDialog
                        title: qsTr("Choose a file to attach")
                        folder: shortcuts.home
                        selectFolder: false
                        selectExisting: true
                        selectMultiple: true
                        onAccepted: {
                            for (var i = 0; i < fileDialog.fileUrls.length; ++i) {
                                controller.attachments.add({url: fileDialog.fileUrls[i]})
                            }
                        }
                    }
                }
            }

            Kube.HeaderField {
                id: subject
                objectName: "subject"
                Layout.fillWidth: true
                activeFocusOnTab: true
                focus: true

                placeholderText: qsTr("Subject")
                text: controller.subject
                onTextChanged: controller.subject = text;
                onActiveFocusChanged: {
                    if (activeFocus) {
                        root.focusChange()
                    }
                }
                onAccepted: textEditor.forceActiveFocus(Qt.TabFocusReason)
            }

            Kube.TextEditor {
                id: textEditor
                objectName: "textEditor"
                activeFocusOnTab: true

                Layout.fillWidth: true
                Layout.fillHeight: true

                border.width: 0

                onHtmlEnabledChanged: {
                    controller.htmlBody = htmlEnabled;
                }

                onActiveFocusChanged: root.focusChange()
                Keys.onEscapePressed: root.done()
                onTextChanged: {
                    controller.body = text;
                }
            }
        }
    }
}
