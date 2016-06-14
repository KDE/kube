/*
 *   Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.theme 1.0

Rectangle {
    id: root

    color: ColorPalette.border

    opacity: 0.9

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.destroy()
        }
    }

    Rectangle {
        anchors.centerIn: parent

        height: root.height * 0.8
        width: root.width * 0.8

        color: ColorPalette.background

        MouseArea {
            anchors.fill: parent
        }

        KubeSettings.Settings {
            id: contextSettings
            identifier: "applicationcontext"
            property string currentAccountId: "current"
        }

        KubeFramework.AccountsController {
            id: accountsController
        }
        KubeFramework.AccountsModel {
            id: accountsModel
        }

        SplitView {
            anchors.fill: parent

            ColumnLayout {
                ScrollView {
                    id: accountsList

                    width: Unit.size * 55
                    Layout.maximumWidth: Unit.size * 150
                    Layout.minimumWidth: Unit.size * 30

                    ListView {
                        id: listView

                        model: accountsModel

                        currentIndex: -1

                        onCountChanged: {
                            listView.currentIndex = count - 1
                        }

                        delegate:  Kirigami.AbstractListItem {
                            enabled: true
                            supportsMouseEvents: true
                            checked: listView.currentIndex == index

                            onCheckedChanged: {
                                if (checked) {
                                    console.warn("Current index changed ", accountFactory.accountId);
                                    accountDetails.source = accountFactory.uiPath
                                    accountDetails.item.accountId = accountFactory.accountId
                                    accountDetails.item.icon = accountFactory.icon
                                    accountDetails.item.accountName = accountFactory.name
                                }
                            }

                            onClicked: {
                                listView.currentIndex = model.index
                            }

                            contentItem: RowLayout {

                                KubeFramework.AccountFactory {
                                    id: accountFactory
                                    accountId: model.accountId
                                    onAccountLoaded: {
                                        if (listView.currentIndex == model.index) {
                                            accountDetails.source = accountFactory.uiPath
                                            accountDetails.item.accountId = accountFactory.accountId
                                            accountDetails.item.icon = accountFactory.icon
                                            accountDetails.item.accountName = accountFactory.name
                                        }
                                    }
                                }

                                Kirigami.Icon {
                                    source: accountFactory.icon
                                }

                                Label {
                                    text: model.name === "" ? accountFactory.name : model.name
                                }
                            }
                        }
                    }

                }
                Button {
                    id: button
                    text: "Create New"
                    onClicked: {
                        accountsController.createAccount("maildir");
                    }
                }
            }

            Loader {
                id: accountDetails

                Layout.fillWidth: true
            }
        }
    }
}
