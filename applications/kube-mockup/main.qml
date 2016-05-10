/*
* Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.2

import org.kde.kirigami 1.0 as Kirigami

Controls.ApplicationWindow {
    id: app

    //FIXME remove fixed pixel hight
    //for now just convinience during testing
    width: 1920/ 1.2
    height: (1080 -40) / 1.2

    Controls.ToolBar {
        id: toolbar

        anchors {
            top: app.top
            left: applications.right
        }

        height: Kirigami.Units.iconSizes.large
        width: app.width - applications.width

        RowLayout {
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            width: Kirigami.Units.gridUnit * 20

            Controls.TextField {

                Layout.fillWidth: true

                placeholderText: "Search"
            }

            Controls.ToolButton {
                iconName: "application-menu"
            }
        }
    }


    Rectangle {
        id: applications

        height: parent.height
        width: Kirigami.Units.iconSizes.large + Kirigami.Units.gridUnit * 2

        color: Kirigami.Theme.viewBackgroundColor

        ColumnLayout{

            anchors {
                top: parent.top
            }

            width: parent.width

            Repeater {

                model: ListModel {
                    ListElement {
                        icon: "kopete"
                        name: "Channels"
                    }
                    ListElement {
                        icon: "kmail"
                        name: "Mail"
                    }
                    ListElement {
                        icon: "korganizer"
                        name: "Calendar"
                    }
                    ListElement {
                        icon: "kaddressbook"
                        name: "Contacts"
                    }
                }

                delegate: Item {

                    height: Kirigami.Units.iconSizes.large + Kirigami.Units.gridUnit
                    width: Kirigami.Units.iconSizes.large + Kirigami.Units.gridUnit * 2

                    Controls.ToolButton {

                        anchors.horizontalCenter: parent.horizontalCenter

                        iconName: model.icon

                        //enabled: false
                    }

                    Kirigami.Label {

                        anchors {
                            bottom: parent.bottom
                            horizontalCenter: parent.horizontalCenter
                        }

                        text: model.name
                        color: Kirigami.Theme.viewTextColor

                        opacity: 0.5
                    }
                }
            }
        }
    }

    Controls.SplitView {

        anchors {
            top: toolbar.bottom
            left: applications.right
        }

        height: app.height - toolbar.height
        width: app.width - applications.width

        FolderPage {
            id: folders

            height: parent.height
            width: 250
        }

        Item {
            id: mails

            height: parent.height
            width: 400

            MailListPage {
                anchors.fill: parent
            }
        }

        SingleMailPage {
            id: conversations

            height: parent.height
            Layout.fillWidth: true
        }
    }
}
