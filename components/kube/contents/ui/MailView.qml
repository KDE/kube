/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
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


import QtQuick 2.7
import QtQuick.Controls 1.3
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

FocusScope {
    SplitView {
        anchors.fill: parent
        Rectangle {
            width: Kube.Units.gridUnit * 10
            Layout.minimumWidth: Kube.Units.gridUnit * 5

            color: Kube.Colors.textColor

            Kube.PositiveButton {
                id: newMailButton

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                focus: true
                text: qsTr("New Email")
                onClicked: Kube.Fabric.postMessage(Kube.Messages.compose, {})
            }

            Kube.InlineAccountSwitcher {
                id: accountFolderview
                activeFocusOnTab: true
                anchors {
                    top: newMailButton.bottom
                    topMargin: Kube.Units.largeSpacing
                    bottom: statusBarContainer.top
                    left: newMailButton.left
                    right: parent.right
                }
            }

            Item {
                id: statusBarContainer
                anchors {
                    topMargin: Kube.Units.smallSpacing
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                height: childrenRect.height

                Rectangle {
                    id: border
                    visible: statusBar.visible
                    anchors {
                        right: parent.right
                        left: parent.left
                        margins: Kube.Units.smallSpacing
                    }
                    height: 1
                    color: Kube.Colors.viewBackgroundColor
                    opacity: 0.3
                }
                Kube.StatusBar {
                    id: statusBar
                    accountId: accountFolderview.currentAccount
                    height: Kube.Units.gridUnit * 2
                    anchors {
                        top: border.bottom
                        left: statusBarContainer.left
                        right: statusBarContainer.right
                    }
                }
            }
        }

        Kube.MailListView  {
            id: mailListView
            width: Kube.Units.gridUnit * 20
            height: parent.height
            activeFocusOnTab: true
            Layout.minimumWidth: Kube.Units.gridUnit * 10
        }

        Kube.ConversationView {
            id: mailView
            Layout.fillWidth: true
            Layout.minimumWidth: Kube.Units.gridUnit * 5
            activeFocusOnTab: true
        }
    }
}
