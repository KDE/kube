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
import QtQuick.Controls 1.3 as Controls1
import QtQuick.Controls 2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.View {
    id: root
    property string searchTerm: ""


    Rectangle {
        id: overlay

        parent: ApplicationWindow.overlay
        height: app.height
        width: app.width - app.sidebarWidth
        x: app.sidebarWidth
        y: 0

        visible: root.searchTerm == ""

        Row {
            anchors.centerIn: parent

            spacing: Kube.Units.smallSpacing

            Kube.TextField {
                id: searchField
                anchors.verticalCenter: parent.verticalCenter
                width: Kube.Units.gridUnit * 30
                focus: true
                text: root.searchTerm
                Keys.onEscapePressed: root.searchTerm = ""
                onAccepted: root.searchTerm = text
            }

            Kube.PositiveButton {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Search")
                enabled: searchField.text != ""
                onClicked: root.searchTerm = searchField.text
            }
        }
    }

    RowLayout {
        spacing: 0
        Rectangle {
            Layout.fillHeight: true
            width: Kube.Units.gridUnit * 10
            color: Kube.Colors.darkBackgroundColor
            ColumnLayout {
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: Kube.Units.smallSpacing
                }
                RowLayout {
                    Kube.CheckBox {
                        checked: false
                    }
                    Kube.Label {
                        text: qsTr("Unread")
                        color: Kube.Colors.highlightedTextColor
                    }
                }
                RowLayout {
                    Kube.CheckBox {
                        checked: false
                    }
                    Kube.Label {
                        text: qsTr("Important")
                        color: Kube.Colors.highlightedTextColor
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                Rectangle {
                    id: toolbar
                    Layout.fillWidth: true
                    height: searchBar.height + Kube.Units.smallSpacing * 2
                    color: Kube.Colors.backgroundColor
                    Kube.TextField {
                        id: searchBar
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        text: root.searchTerm
                        width: parent.width * 0.5
                        placeholderText: qsTr("Search...")
                        Keys.onEscapePressed: root.searchTerm = ""
                        onTextChanged: {
                            forceActiveFocus()
                            mailListView.filter = text
                        }
                    }
                }

                Controls1.SplitView {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Kube.MailListView  {
                        id: mailListView
                        width: Kube.Units.gridUnit * 18
                        Layout.minimumWidth: Kube.Units.gridUnit * 10
                        Layout.fillHeight: true
                    }
                    Kube.ConversationView {
                        id: mailView
                        Layout.minimumWidth: Kube.Units.gridUnit * 5
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        activeFocusOnTab: true
                        model: Kube.MailListModel {
                            singleMail: mailListView.currentMail
                        }
                    }
                }
            }
        }
    }
}
