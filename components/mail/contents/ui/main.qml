/*
  Copyright (C) 2015 Michael Bohlender, <michael.bohlender@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


import QtQuick 2.7
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

import QtQuick.Controls 2.0 as Controls2
import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.actions 1.0 as KubeAction
import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.components 1.0 as KubeComponents
import org.kube.accounts 1.0 as KubeAccounts

Controls2.ApplicationWindow {
    id: app

    //FIXME remove fixed pixel hight
    //for now just convinience during testing
    height: 1080 * 0.7
    width: 1920  * 0.7

    visible: true

    //BEGIN Actions
    KubeAction.Context {
        id: maillistcontext
        property variant mail
        property bool isDraft
        mail: mailListView.currentMail
        isDraft: mailListView.isDraft
    }

    KubeAction.Action {
        id: replyAction
        actionId: "org.kde.kube.actions.reply"
        context: maillistcontext
    }
    //END Actions

    //BEGIN ActionHandler
    KubeAction.ActionHandler {
        actionId: "org.kde.kube.actions.reply"
        function isReady(context) {
            return context.mail ? true : false;
        }

        function handler(context) {
            composer.loadMessage(context.mail, false)
            composer.open()
        }
    }

    KubeAction.ActionHandler {
        actionId: "org.kde.kube.actions.edit"
        function isReady(context) {
            return context.mail && context.isDraft;
        }
        function handler(context) {
            composer.loadMessage(context.mail, true)
            composer.open()
        }
    }
    //END ActionHandler

    //Controller
    KubeFramework.MailController {
        id: mailController
        mail: mailListView.currentMail
    }

    KubeFramework.FolderController {
        id: folderController
        folder: folderListView.currentFolder
    }


    //BEGIN Shortcuts
    Shortcut {
        sequence: StandardKey.Refresh
        onActivated: folderController.synchronizeAction.execute()
        enabled: folderController.synchronizeAction.enabled
    }
    Shortcut {
        sequence: StandardKey.Delete
        onActivated: mailController.moveToTrashAction.execute()
        enabled: mailController.moveToTrashAction.enabled
    }
    Shortcut {
        sequence: StandardKey.MoveToNextLine
        onActivated: mailListView.currentIndex++
    }
    Shortcut {
        sequence: StandardKey.MoveToPreviousLine
        onActivated: mailListView.currentIndex--
    }
    //END Shortcuts

    //BEGIN background
    Rectangle {
        anchors.fill: parent

        color: Kirigami.Theme.backgroundColor
    }
    //END background

    //BEGIN Main content
    SplitView {
        anchors {
            top: toolbar.bottom
            left: app.left
        }

        height: app.height - toolbar.height
        width: app.width

        KubeComponents.FolderListView {
            id: folderListView
            width: Kirigami.Units.gridUnit * 10
            Layout.maximumWidth: app.width * 0.25
            Layout.minimumWidth: Kirigami.Units.gridUnit * 5
            focus: true
            accountId: accountSwitcher.accountId
        }

        KubeComponents.MailListView  {
            id: mailListView
            parentFolder: folderListView.currentFolder
            width: Kirigami.Units.gridUnit * 20
            height: parent.height
            Layout.maximumWidth: app.width * 0.4
            Layout.minimumWidth: Kirigami.Units.gridUnit * 10
            focus: true
        }

        KubeComponents.SingleMailView {
            id: mailView
            mail: mailListView.currentMail
            Layout.fillWidth: true
        }
    }
    //END Main content

    //BEGIN Toolbar
    Controls2.ToolBar {
        id: toolbar

        anchors {
            top: app.top
        }

        width: app.width
        height: Kirigami.Units.iconSizes.medium + Kirigami.Units.smallSpacing * 2

        Row {
            width: parent.width
            height: parent.height

            spacing: 1 //to account for the SplitView borders

            //BEGIN Folderlist section
            RowLayout {
                height: parent.height
                width: folderListView.width - 5 //to adjust for the toolbar spacing

                KubeComponents.AccountSwitcher {
                    id: accountSwitcher
                }
            }
            //END Folderlist section

            //BEGIN MailList section
            Item {

                height: parent.height
                width: mailListView.width

                Row {
                    anchors.centerIn: parent

                    spacing: Kirigami.Units.smallSpacing

                    ToolButton {
                        iconName: "mail-mark-unread"
                        text: qsTr("Mark As Read")
                        enabled: mailController.markAsReadAction.enabled
                        tooltip: qsTr("mark mail as read")
                        onClicked: {
                            mailController.markAsReadAction.execute()
                        }
                    }

                    ToolButton {
                        iconName: "mail-mark-important"
                        text: qsTr("Mark Important")
                        enabled: mailController.markAsImportantAction.enabled
                        tooltip: qsTr("mark mail as important")
                        onClicked: {
                            mailController.markAsImportantAction.execute()
                        }
                    }

                    ToolButton {
                        iconName: "edit-delete"
                        text: qsTr("Delete Mail")
                        enabled: mailController.moveToTrashAction.enabled
                        tooltip: qsTr("delete email")
                        onClicked: {
                            mailController.moveToTrashAction.execute()
                        }
                    }

                    ToolButton {
                        iconName: "edit-undo"
                        text: qsTr("Restore Mail")
                        enabled: mailController.restoreFromTrashAction.enabled
                        tooltip: qsTr("restore email")
                        onClicked: {
                            mailController.restoreFromTrashAction.execute()
                        }
                    }
                }
            }
            //END MailList section

            //BEGIN MailView sections
            RowLayout{

                height: parent.height
                width: mailView.width

                Item {
                    width: Kirigami.Units.smallSpacing
                    height: width
                }

                Controls2.Button {
                    id: newMailButton


                    height: toolbar.height

                    //iconName: "mail-message-new"
                    text: "      " + qsTr("New Email") + "      "
                    //Controls2.Tooltip.text: "compose new email"
                    onClicked: {
                        composer.open()
                    }
                }

                KubeComponents.Outbox {
                }
/*
                ToolButton {
                    iconName: "mail-message-reply"
                    text: "Reply"
                    enabled: replyAction.ready
                    onClicked: {
                        replyAction.execute()
                    }
                }


                 *               ToolButton {
                 *                   iconName: "mail-message-edit"
                 *                   text: "Edit"
                 *                   enabled: editAction.ready
                 *                   onClicked: {
                 *                       editAction.execute()
            }
            }
            */

                Item {
                    Layout.fillWidth: true
                }

                Controls2.TextField {
                    id: searchBar

                    Layout.minimumWidth: Kirigami.Units.gridUnit * 19
                    height: toolbar.height

                    placeholderText: "Search..."
                }

                Item  {
                    width: Kirigami.Units.smallSpacing
                }
            }
            //END MailView section
        }
    }
    //END ToolBar

    //BEGIN Composer
    KubeComponents.FocusComposer {
        id: composer

        height: app.height * 0.85
        width: app.width * 0.85

        x: app.width * 0.075
        y: toolbar.y + toolbar.height
    }
    //END Composer

    //BEGIN AccountWizard
    KubeAccounts.AccountWizard {
        id: accountWizard

        // visible: true

        height: app.height * 0.85
        width: app.width * 0.85

        x: app.width * 0.075
        y: 50
    }
    //END AccountWizard
}
