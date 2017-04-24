/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Controls 1.4 as Controls

import QtQml 2.2 as QtQml

import org.kde.kirigami 1.0 as Kirigami
import org.kube.framework 1.0 as Kube


Controls.ToolButton {
    id: accountSwitcher

    property variant accountId
    property variant accountName

    width: parent.width
    iconName: Kube.Icons.menu_inverted

    Kube.AccountsModel {
        id: accountsModel
    }

    onClicked: {
        popup.open()
    }

    Kube.Popup {
        id: popup

        height: listView.count == 0 ? Kirigami.Units.gridUnit * 4 : Kirigami.Units.gridUnit * 2 + listView.count * Kirigami.Units.gridUnit * 3
        width: Kirigami.Units.gridUnit * 20

        y: accountSwitcher.y - height

        Item {
            id: buttons
            anchors {
                bottom: parent.bottom
            }

            height: Kirigami.Units.gridUnit * 2
            width: parent.width

            Kube.Button {
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                }

                //iconName: "view-refresh"
                text: "Sync"
                onClicked: {
                    Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": accountSwitcher.accountId})
                    popup.close()
                }
            }

            Kube.PositiveButton {
                id: newAccountButton

                anchors {
                    right: parent.right
                    bottom: parent.bottom
                }

                text: "Create new Account"

                onClicked: {
                    accountWizard.open()
                    popup.close()
                }
            }
        }

        ListView {
            id: listView

            anchors {
                top: parent.top
                bottom: buttons.top
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

                        Kube.Label {
                            text: model.name
                        }

                        Controls.ToolButton {
                            id: statusIcon
                            visible: false
                            iconName: ""
                            enabled: false
                            states: [
                                State {
                                    name: "busy"; when: model.status == Kube.AccountsModel.BusyStatus
                                    PropertyChanges { target: statusIcon; iconName: Kube.Icons.busy; visible: true }
                                },
                                State {
                                    name: "error"; when: model.status == Kube.AccountsModel.ErrorStatus
                                    PropertyChanges { target: statusIcon; iconName: Kube.Icons.error; visible: true }
                                },
                                State {
                                    name: "checkmark"; when: model.status == Kube.AccountsModel.ConnectedStatus
                                    PropertyChanges { target: statusIcon; iconName: Kube.Icons.connected; visible: true }
                                }
                            ]
                        }
                    }
                    Kube.Button {

                        anchors {
                            right: parent.right
                            rightMargin: Kirigami.Units.largeSpacing
                            verticalCenter: parent.verticalCenter
                        }

                        opacity: hovered ? 1 : 0.7
                        visible: accountDelegate.containsMouse
                        text: "edit"

                        onClicked: {
                            editAccountComponent.createObject(app, {accountId:model.accountId})
                            popup.close()
                        }

                        Component {
                            id: editAccountComponent
                            Kube.EditAccountDialog {
                                anchors.fill: parent
                            }
                        }
                    }
                }
            }
        }
    }
}
