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
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

FocusScope {
    Controls1.SplitView {
        anchors.fill: parent
        ColumnLayout {
            width: Kube.Units.gridUnit * 18
            Layout.minimumWidth: Kube.Units.gridUnit * 10
            Rectangle {
                id: filterField
                Layout.fillWidth: true
                height: Kube.Units.gridUnit * 2
                color: Kube.Colors.darkBackgroundColor

                function clearSearch() {
                    find.text = ""
                    mailListView.filter = ""
                }

                RowLayout {
                    anchors {
                        verticalCenter: parent.verticalCenter
                    }

                    width: parent.width - Kube.Units.smallSpacing
                    spacing: 0

                    Kube.IconButton {
                        iconName: Kube.Icons.remove
                        onClicked: filterField.clearSearch()
                    }

                    Kube.TextField {
                        id: find
                        Layout.fillWidth: true
                        placeholderText: qsTr("Search...")
                        onTextChanged: mailListView.filter = text
                        focus: true
                        Keys.onEscapePressed: filterField.clearSearch()
                    }
                }
            }
            Kube.MailListView  {
                id: mailListView
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
        Kube.ConversationView {
            id: mailView
            objectName: "mailView"
            Layout.fillWidth: true
            Layout.fillHeight: parent.height
            activeFocusOnTab: true
            mail: mailListView.currentMail
        }
    }
}
