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
        mail: mailListView.currentMail
    }

    KubeAction.Context {
        id: folderListContext
        property variant folder
        folder: folderListView.currentFolder
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

    Item {
        id: toolbar
        anchors {
            top: app.top
            left: app.left
        }

        height: Kirigami.Units.iconSizes.medium
        width: app.width

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "grey"
        }

        Row {
            anchors.fill: parent

            spacing: 1 //to account for the SplitView borders

            RowLayout {
                height: parent.height
                width: folderListView.width
                clip: true

                KubeComponents.AccountSwitcher {
                    Layout.fillHeight: true

                }

                ToolButton {
                    Layout.fillHeight: true
                    iconName: "view-refresh"
                    text: "Sync"
                    enabled: syncAction.ready

                    onClicked: {
                        syncAction.execute()
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                height: parent.height
                width: mailListView.width
                clip: true

                ToolButton {
                    Layout.fillHeight: true
                    iconName: "mail-mark-unread"
                    text: "Mark As Read"
                    enabled: markAsReadAction.ready
                    onClicked: {
                        markAsReadAction.execute()
                    }
                }

                ToolButton {
                    Layout.fillHeight: true
                    iconName: "mail-mark-important"
                    text: "Mark Important"
                    enabled: false
                    onClicked: {
                    }
                }

                ToolButton {
                    Layout.fillHeight: true
                    iconName: "edit-delete"
                    text: "Delete Mail"
                    enabled: deleteAction.ready
                    onClicked: {
                        deleteAction.execute()
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }


            RowLayout{
                height: parent.height
                width: mailView.width
                clip: true

                Component {
                    id: composerComponent

                    KubeComponents.FocusComposer {
                        id: composer
                        anchors.fill: parent
                    }
                }

                ToolButton {
                    id: newMailButton

                    Layout.fillHeight: true

                    iconName: "mail-message-new"
                    text: "Compose"
                    onClicked: {
                        composerComponent.createObject(app)
                    }
                }

                ToolButton {
                    Layout.fillHeight: true
                    iconName: "mail-message-reply"
                    text: "Reply"
                    onClicked: {
                        composerComponent.createObject(app)
                        composer.originalMessage = mailListView.currentMail
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                TextField {
                    id: searchBar

                    Layout.minimumWidth: Kirigami.Units.gridUnit * 25

                    placeholderText: "Search..."
                }

                ToolButton {
                    id: settingsButton

                    Layout.fillHeight: true

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

