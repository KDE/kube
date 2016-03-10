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

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.framework.domain 1.0 as KubeFramework

Rectangle {
    id: root

    visible: false

    color: colorPalette.border

    opacity: 0.9

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.visible = false
        }
    }

    Rectangle {
        anchors.centerIn: parent

        height: root.height * 0.8
        width: root.width * 0.8

        color: colorPalette.background

        MouseArea {
            anchors.fill: parent
        }

        KubeSettings.Settings {
            id: contextSettings
            identifier: "applicationcontext"
            property string currentAccountId: "current"
            property var availableAccountTypes: ["maildir"]
            property var accounts: ["current"]
        }

        Column {
            spacing: 5
            Repeater {
                model: contextSettings.accounts
                delegate: Rectangle {
                    KubeFramework.AccountFactory {
                        id: accountFactory
                        accountId: modelData
                    }
                    PlasmaCore.IconItem {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            // leftMargin: unit.size * 3
                        }
                        source: accountFactory.icon
                    }
                    Label {
                        text: accountFactory.name
                    }
                    Loader { source: accountFactory.uiPath }
                }
            }
        }

        //TODO: Add possibility to add more accounts
    }
}
