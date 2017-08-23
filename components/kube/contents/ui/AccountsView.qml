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
import QtQuick.Controls 1.3 as Controls
import org.kube.framework 1.0 as Kube
import org.kube.components.accounts 1.0 as KubeAccounts

Rectangle {
    color: Kube.Colors.backgroundColor

    Controls.SplitView {
        height: parent.height
        width: parent.width

        Item {
            id: accountList
            width: Kube.Units.gridUnit * 12
            Layout.fillHeight: true

            Kube.PositiveButton {
                id: newAccountButton
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                text: qsTr("New Account")

                onClicked: accountWizard.open()
            }

            Kube.ListView {
                id: listView

                anchors {
                    top: newAccountButton.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    topMargin: Kube.Units.largeSpacing
                }

                clip: true

                model: Kube.AccountsModel {}

                onCurrentItemChanged: {
                    if (currentItem) {
                        edit.accountId = currentItem.currentData.accountId
                    }
                }

                delegate: Kube.ListDelegate {
                    id: delegateRoot

                    Kube.Label {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: Kube.Units.largeSpacing
                        }
                        width: parent.width - Kube.Units.largeSpacing * 2

                        text: model.name
                        color: delegateRoot.textColor
                    }
                }
            }
        }

        Item {
            height: parent.height
            width: Kube.Units.gridUnit * 20
            Layout.fillWidth: true

            Kube.EditAccount {
                id: edit
                anchors {
                    fill: parent
                    bottomMargin: Kube.Units.largeSpacing
                }

            }
        }
    }

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
