/*
 * Copyright (C) 2015 Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.actions 1.0 as KubeAction
import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.framework.theme 1.0
import org.kube.components 1.0 as KubeComponents

ApplicationWindow {
    id: app

    //FIXME remove fixed pixel hight
    //for now just convinience during testing
    height: 1080 * 0.7
    width: 1920  * 0.7

    visible: true

    KubeAction.Context {
        id: maillistcontext
        property variant mail
        property bool isDraft
        mail: mailListView.currentMail
        isDraft: mailListView.isDraft
    }

    KubeAction.Context {
        id: folderListContext
        property variant folder
        folder: folderListView.currentFolder
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

    KubeAction.Action {
        id: markAsReadAction
        actionId: "org.kde.kube.actions.mark-as-read"
        context: maillistcontext
    }

    KubeAction.Action {
        id: deleteAction
        actionId: "org.kde.kube.actions.delete"
        context: maillistcontext
    }

    KubeAction.Action {
        id: syncAction
        actionId: "org.kde.kube.actions.synchronize"
        context: folderListContext
    }

    ToolBar {
        id: toolbar
        anchors {
            top: app.top
            left: app.left
            right: app.right
        }

        height: Kirigami.Units.iconSizes.medium
        //width: app.width

        Row {
            anchors.fill: parent

            spacing: 1 //to account for the SplitView borders

            RowLayout {
                height: parent.height
                width: folderListView.width - 5 //to adjust for the toolbar spacing
                clip: true

                KubeComponents.AccountSwitcher {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                ToolButton {
                    height: toolbar.height
                    width: toolbar.width
                    iconName: "view-refresh"
                    text: "Sync"
                    enabled: syncAction.ready

                    onClicked: {
                        syncAction.execute()
                    }
                }
            }

            Item {
                height: parent.height
                width: mailListView.width
                clip: true

                RowLayout {
                    anchors.centerIn: parent

                    ToolButton {
                        height: toolbar.height
                        width: toolbar.width
                        iconName: "mail-mark-unread"
                        text: "Mark As Read"
                        enabled: markAsReadAction.ready
                        onClicked: {
                            markAsReadAction.execute()
                        }
                    }

                    ToolButton {
                        height: toolbar.height
                        width: toolbar.width
                        iconName: "mail-mark-important"
                        text: "Mark Important"
                        enabled: false
                        onClicked: {
                        }
                    }

                    ToolButton {
                        height: toolbar.height
                        width: toolbar.width
                        iconName: "edit-delete"
                        text: "Delete Mail"
                        enabled: deleteAction.ready
                        onClicked: {
                            deleteAction.execute()
                        }
                    }
                }
            }

            RowLayout{
                height: parent.height
                width: mailView.width - 5 //to adjust for the toolbar spacing
                clip: true

                Component {
                    id: composerComponent

                    KubeComponents.FocusComposer {
                        id: composer
                        anchors.fill: parent
                    }
                }

                KubeAction.ActionHandler {
                    actionId: "org.kde.kube.actions.reply"
                    function isReady(context) {
                        return context.mail ? true : false;
                    }

                    function handler(context) {
                        var component = composerComponent.createObject(app)
                        component.loadMessage(context.mail, false)
                    }
                }

                KubeAction.ActionHandler {
                    actionId: "org.kde.kube.actions.edit"
                    function isReady(context) {
                        return context.mail && context.isDraft;
                    }

                    function handler(context) {
                        var component= composerComponent.createObject(app, {"draftMessage": context.mail})
                        component.loadMessage(context.mail, true)
                    }
                }

                ToolButton {
                    id: newMailButton

                    height: toolbar.height
                    width: toolbar.width

                    iconName: "mail-message-new"
                    text: "Compose"
                    onClicked: {
                        composerComponent.createObject(app)
                    }
                }

                ToolButton {
                    height: toolbar.height
                    width: toolbar.width
                    iconName: "mail-message-reply"
                    text: "Reply"
                    enabled: replyAction.ready
                    onClicked: {
                        replyAction.execute()
                    }
                }

                ToolButton {
                    height: toolbar.height
                    width: toolbar.width
                    iconName: "mail-message-edit"
                    text: "Edit"
                    enabled: editAction.ready
                    onClicked: {
                        editAction.execute()
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                TextField {
                    id: searchBar

                    Layout.minimumWidth: Kirigami.Units.gridUnit * 25
                    height: toolbar.height

                    placeholderText: "Search..."
                }

                ToolButton {
                    id: settingsButton

                    height: toolbar.height
                    width: toolbar.width

                    iconName: "application-menu"
                    text: "Settings"

                    onClicked: {
                        settingsComponent.createObject(app)
                    }

                    Component {
                        id: settingsComponent
                        KubeComponents.Settings {
                            id: settings
                            anchors.fill: parent
                        }
                    }
                }
            }
        }
    }

    SplitView {
        anchors {
            top: toolbar.bottom
            left: app.left
        }

        height: app.height - toolbar.height
        width: app.width

        KubeComponents.FolderListView {
            id: folderListView
            width: Unit.size * 55
            Layout.maximumWidth: Unit.size * 150
            Layout.minimumWidth: Unit.size * 30
        }

        KubeComponents.MailListView  {
            id: mailListView
            parentFolder: folderListView.currentFolder
            width: Unit.size * 80
            Layout.maximumWidth: Unit.size * 250
            Layout.minimumWidth: Unit.size * 50
            focus: true
        }

        KubeComponents.SingleMailView {
            id: mailView
            mail: mailListView.currentMail
            Layout.fillWidth: true
        }

    }

}

