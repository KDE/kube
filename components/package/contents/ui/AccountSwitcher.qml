/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Controls 1.4 as Controls

import QtQml 2.2 as QtQml

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.accounts 1.0 as KubeAccounts
import org.kube.components 1.0 as KubeComponents

Item {
    id: accountSwitcher

    property variant accountId
    property variant accountName

    width: parent.width

    clip: true

    KubeFramework.FolderController {
        id: folderController
    }

    KubeAccounts.AccountsModel {
        id: accountsModel
    }

    Text {
        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.smallSpacing
            bottom: parent.bottom
        }

        text: accountName
        color: Kirigami.Theme.backgroundColor
        font.weight: Font.DemiBold
    }

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (mouse.button == Qt.RightButton) {
                contextMenu.popup()
            } else {
                popup.open()
                focus = false
            }
        }
    }

    Controls.Menu {
        id: contextMenu
        title: "Edit"

        Controls.MenuItem {
            text: "Synchronize"
            onTriggered: {
                folderController.synchronizeAction.execute()
            }
        }
    }

    Controls2.Popup {
        id: popup

        height: listView.count == 0 ? Kirigami.Units.gridUnit * 2  : Kirigami.Units.gridUnit * 2 + listView.count * Kirigami.Units.gridUnit * 3
        width: parent.width

        modal: true
        focus: true
        closePolicy: Controls2.Popup.CloseOnEscape | Controls2.Popup.CloseOnPressOutsideParent


//             Controls2.Button {
//                 anchors {
//                     verticalCenter: parent.verticalCenter
//                     left: parent.left
//                 }
//
//                 //iconName: "view-refresh"
//                 text: "Sync"
//                 enabled: folderController.synchronizeAction.enabled
//                 onClicked: {
//                     folderController.synchronizeAction.execute()
//                     popup.close()
//                 }
//             }

            Controls2.Button {
                id: newAccountButton

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                }

                text: "Create new Account"

                onClicked: {
                    accountWizard.open()
                    popup.close()
                }
            }

        ListView {
            id: listView

            anchors {
                top: parent.top
                bottom: newAccountButton.top
                left: parent.left
                right: parent.right
            }

            clip: true

            model: accountsModel

            delegate: Kirigami.AbstractListItem {
                id: accountDelegate

                height: Kirigami.Units.gridUnit * 2

                enabled: true
                supportsMouseEvents: true

                checked: listView.currentIndex == index
                onClicked:  {
                    listView.currentIndex = model.index
                    popup.close()
                }
                Item {
                    height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 1
                    width: listView.width

                    QtQml.Binding {
                        target: accountSwitcher
                        property: "accountId"
                        when: listView.currentIndex == index
                        value: model.accountId
                    }

                    QtQml.Binding {
                        target: accountSwitcher
                        property: "accountName"
                        when: listView.currentIndex == index
                        value: model.name
                    }

                    RowLayout {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            margins: Kirigami.Units.smallSpacing
                        }

                        Layout.fillHeight: true

                        KubeAccounts.AccountFactory {
                            id: accountFactory
                            accountId: model.accountId
                        }

//                         Kirigami.Icon {
//                             source: model.icon
//                         }

                        Controls2.Label {
                            text: model.name
                        }

                        Controls.ToolButton {
                            visible: model.showStatus
                            iconName: model.statusIcon
                        }
                    }
                    Controls2.Button {

                        anchors {
                            right: parent.right
                            rightMargin: Kirigami.Units.largeSpacing
                            verticalCenter: parent.verticalCenter
                        }

                        opacity: hovered ? 1 : 0.7
                        visible: accountDelegate.containsMouse
                        text: "edit"

                        onClicked: {
                            editAccountComponent.createObject(app)
                            popup.close()
                        }

                        Component {
                            id: editAccountComponent

                            KubeComponents.EditAccountDialog {
                                id: editAccount

                                anchors.fill: parent

                                accountId: accountFactory.accountId
                                uiSource: accountFactory.uiPath
                            }
                        }
                    }
                }
            }
        }
    }
}
