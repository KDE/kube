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

import QtQuick.Controls 1.4 as Controls1
import QtQuick.Controls 2.0 as Controls2

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.components 1.0 as KubeComponents

Controls2.Button {
    id: accountSwitcher

    text: "Account Switcher"
    //ToolTip.text: "switch accounts, edit them and add new ones"

    onClicked: {
        //dialog.visible = dialog.visible ? false : true
        onClicked: popup.open()
    }

    Controls2.Popup {
        id: popup

        width: 600
        height: 300
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                ListView {
            id: listView

            anchors {
                top: parent.top
                bottom: footer.top
                left: parent.left
                right: parent.right
                margins: Kirigami.Units.smallSpacing
            }

            clip: true

            model: KubeFramework.AccountsModel {  }

            delegate: Kirigami.AbstractListItem {
                id: accountDelegate

                enabled: true
                supportsMouseEvents: true

                 RowLayout {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            margins: Kirigami.Units.smallSpacing
                        }

                        width: listView.width
                        height: Kirigami.Units.gridUnit * 1.5

                        KubeFramework.AccountFactory {
                            id: accountFactory
                            accountId: model.accountId
                        }

                        RowLayout {

                        Kirigami.Icon {
                            source: accountFactory.icon
                        }

                        Controls2.Label {
                            text: model.name === "" ? accountFactory.name : model.name
                        }
                        Controls1.ToolButton {
                            visible: model.showStatus
                            iconName: model.statusIcon
                        }

                        }

                    Controls2.Button {

                        anchors {
                            right: parent.right
                            margins: Kirigami.Units.largeSpacing
                        }

                        opacity: hovered ? 1 : 0.7
                        visible: accountDelegate.containsMouse
                        text: "edit"

                        onClicked: {
                            editAccountComponent.createObject(app)
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

         Item {
            id: footer

            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: Kirigami.Units.largeSpacing
            }

            height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 1
            width: listView.width

            Controls2.Button {

                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                }

                text: "Create new Account"

                onClicked: {
                    newAccountComponent.createObject(app)
                }

                Component {
                    id: newAccountComponent
                    KubeComponents.NewAccountDialog {
                        id: settings
                        anchors.fill: parent
                    }
                }
            }

            Controls2.Button {

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                }

                //iconName: "view-refresh"
                text: "Sync"
                enabled: syncAction.ready

                onClicked: {
                    syncAction.execute()
                }
            }
         }
    }
}


