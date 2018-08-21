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
    property string currentAccount
    onCurrentAccountChanged: {
        Kube.Fabric.postMessage(Kube.Messages.accountSelection, {accountId: currentAccount});
    }

    property var currentFolder

    ColumnLayout {
        id: layout
        anchors.fill: parent

        Repeater {
            model: Kube.AccountsModel {}
            onItemAdded: {
                //Autoselect the first account to appear
                if (!currentAccount) {
                    root.currentAccount = item.currentData.accountId
                }
            }

            delegate: Item {
                id: accountDelegate
                property variant currentData: model
                property bool isCurrent: (model.accountId == root.currentAccount)

                anchors {
                    left: layout.left
                    right: layout.right
                }
                height: Kube.Units.gridUnit
                Layout.fillHeight: isCurrent

                Kube.TextButton {
                    id: accountButton
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        rightMargin: Kube.Units.smallSpacing
                    }
                    height: Kube.Units.gridUnit

                    textColor: Kube.Colors.highlightedTextColor
                    activeFocusOnTab: !isCurrent
                    hoverEnabled: !isCurrent
                    onClicked: root.currentAccount = model.accountId
                    text: model.name
                    font.weight: Font.Bold
                    font.family: Kube.Font.fontFamily
                    horizontalAlignment: Text.AlignHLeft
                    padding: 0
                }

                Kube.FolderListView {
                    objectName: "folderListView"
                    anchors {
                        top: accountButton.bottom
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    focus: isCurrent
                    activeFocusOnTab: true

                    accountId: currentData.accountId
                    visible: isCurrent
                    onVisibleChanged: {
                        if (visible) {
                            selectRootIndex()
                        }
                    }

                    onActivated: {
                        if (visible) {
                            Kube.Fabric.postMessage(Kube.Messages.folderSelection, {"folder": model.data(index, Kube.FolderListModel.DomainObject),
                                                                                    "trash": model.data(index, Kube.FolderListModel.Trash)})
                            root.currentFolder = model.data(index, Kube.FolderListModel.DomainObject)
                        }
                    }
                }
            }
        }
    }
}
