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

import org.kube.framework 1.0 as Kube

Rectangle {
    id: app

    height: 800
    width: 1400

    color: Kube.Colors.backgroundColor

    RowLayout {

        anchors.fill: parent

        Item {
            id: accountList
            width: 300
            Layout.fillHeight: true

            Kube.PositiveButton {
                id: newAccountButton
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                text: "New Account"
            }

            ListView {
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

                delegate: Rectangle {
                    height: Kube.Units.gridUnit * 3
                    width: listView.width

                    border.color: Kube.Colors.buttonColor
                    border.width: 1
                    color: Kube.Colors.viewBackgroundColor

                    Kube.Label {
                        anchors.centerIn: parent
                        text: model.name
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent

                        onClicked: {
                            console.log("clicked account  \"" + model.accountId + "\"" )
                            //editAccountComponent.createObject(app, {accountId:model.accountId})
                            edit.accountId = model.accountId
                        }
                    }
                }
            }
        }

        Rectangle {
            height: parent.height
            width: 1
            color: Kube.Colors.buttonColor
        }

        Item {

            height: parent.height
            width: 200
            Layout.fillWidth: true

            Kube.EditAccount {
                id: edit

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    margins: Kube.Units.largeSpacing
                }
            }
        }
    }
}
