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

FocusScope {
    id: root
    property Component delegate: null

    ColumnLayout {
        id: layout
        anchors.fill: parent

        Repeater {
            model: Kube.AccountsModel {}
            onItemAdded: {
                //Autoselect the first account to appear
                if (!Kube.Context.currentAccountId) {
                    Kube.Context.currentAccountId = item.currentData.accountId
                }
            }

            delegate: ColumnLayout {
                id: accountDelegate
                property variant currentData: model
                property bool isCurrent: false

                states: [
                    State {
                        name: "current"
                        when: model.accountId == Kube.Context.currentAccountId
                        PropertyChanges {target: accountDelegate; isCurrent: true}
                    }
                ]

                Layout.minimumHeight: Kube.Units.gridUnit
                Layout.fillHeight: isCurrent
                Layout.fillWidth: true

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Kube.Units.gridUnit
                    Kube.TextButton {
                        anchors {
                            left: parent.left
                            top: parent.top
                        }
                        height: Kube.Units.gridUnit
                        width: parent.width - Kube.Units.gridUnit

                        textColor: Kube.Colors.highlightedTextColor
                        activeFocusOnTab: !isCurrent
                        hoverEnabled: !isCurrent
                        onClicked: {
                            Kube.Context.currentAccountId = model.accountId
                        }
                        text: model.name
                        font.weight: Font.Bold
                        font.family: Kube.Font.fontFamily
                        horizontalAlignment: Text.AlignHLeft
                        padding: 0
                    }
                    Loader {
                        anchors {
                            right: parent.right
                            top: parent.top
                        }
                        focus: isCurrent
                        activeFocusOnTab: isCurrent
                        visible: isCurrent
                        sourceComponent: delegateLoader.item.buttonDelegate
                    }
                }

                Loader {
                    id: delegateLoader
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    focus: accountDelegate.isCurrent
                    activeFocusOnTab: true
                    visible: accountDelegate.isCurrent
                    sourceComponent: root.delegate
                    property variant accountId: currentData.accountId
                    property bool isCurrent: accountDelegate.isCurrent
                    onIsCurrentChanged: {
                        if (!!item && 'currentChanged' in item) {
                            item.currentChanged()
                        }
                    }
                }
            }
        }
    }
}
