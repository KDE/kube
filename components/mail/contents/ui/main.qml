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
    height: 1080 * 0.8
    width: 1920  * 0.8

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
            top: app.top
            left: app.left
        }

        height: app.height
        width: app.width

        Rectangle {
            width: Kirigami.Units.gridUnit * 10
            Layout.maximumWidth: app.width * 0.25
            Layout.minimumWidth: Kirigami.Units.gridUnit * 5

            color: Kirigami.Theme.textColor

            Controls2.ToolBar {
                id: toolBar

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }

                RowLayout {
                    anchors.centerIn: parent

                    spacing: Kirigami.Units.largeSpacing

                    ToolButton {
                        iconName: "mail-message"
                        height: Kirigami.Units.gridUnit * 1.5
                        width: height
                    }

                    ToolButton {
                        iconName: "user"
                        height: Kirigami.Units.gridUnit * 1.5
                        width: height
                    }
                }
            }

            Rectangle {
                id: newMailButton

                anchors {
                    top: toolBar.bottom
                    left: parent.left
                    right: parent.right
                    margins: Kirigami.Units.largeSpacing
                }

                color: "#27ae60"
                clip: true

                height: Kirigami.Units.gridUnit * 1.5

                Text {
                    anchors.centerIn: parent

                    text: qsTr("New Email")
                    color: "white"
                }
                //iconName: "mail-message-new"
                //Controls2.Tooltip.text: "compose new email"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        composer.open()
                    }
                }
            }

            KubeComponents.AccountSwitcher {
                id: accountSwitcher

                anchors {
                    top: newMailButton.bottom
                    topMargin: Kirigami.Units.smallSpacing
                }

                width: parent.width
                height: Kirigami.Units.gridUnit * 2
            }

            KubeComponents.FolderListView {
                id: folderListView

                anchors {
                    top: accountSwitcher.bottom
                    topMargin: Kirigami.Units.smallSpacing
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }

                focus: true
                accountId: accountSwitcher.accountId
            }
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

    //BEGIN Composer
    KubeComponents.FocusComposer {
        id: composer

        height: app.height * 0.85
        width: app.width * 0.85

        x: app.width * 0.075
        y: app.height * 0.075
    }
    //END Composer

    //BEGIN AccountWizard
    KubeAccounts.AccountWizard {
        id: accountWizard

        height: app.height * 0.85
        width: app.width * 0.85

        x: app.width * 0.075
        y: app.height * 0.075
    }
    //END AccountWizard
}
