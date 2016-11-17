/*
 * Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4 as Controls
import QtQuick.Controls 2.0 as Controls2
import org.kde.kirigami 1.0 as Kirigami

Item {

    Controls.ToolButton {
        iconName: "go-previous"

        tooltip: "go back"

        onClicked: {
            stack.pop()
        }
    }

    //Item to avoid anchors conflict with stack
    Item {

        anchors {
            fill: parent
            margins: Kirigami.Units.largeSpacing * 2
        }

        Kirigami.Heading {
            id: heading
            text: "Connect your Kolab Now account"

            color: Kirigami.Theme.highlightColor
        }

        Kirigami.Label {
            id: subHeadline

            anchors {
                left: heading.left
                top: heading.bottom
            }

            width: parent.width

            text: "To let Kube access your account, fill in email address, username, password and give the account a title that will be displayed inside Kube."

            color: Kirigami.Theme.disabledTextColor

            wrapMode: Text.Wrap
        }

        GridLayout {
            anchors {
                top:subHeadline.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                topMargin: Kirigami.Units.largeSpacing * 2
            }

            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            Controls.Label {
                text: "Title of Account"
                Layout.alignment: Qt.AlignRight
            }
            Controls.TextField {
                Layout.fillWidth: true

                placeholderText: "E.g. \"Work\", \"Home\" that will be displayed in Kube as name"
            }

            Controls.Label {
                text: "Email address"
                Layout.alignment: Qt.AlignRight
            }

            Controls.TextField {
                Layout.fillWidth: true

                placeholderText: "Your email address"
            }

            Kirigami.Label {
                text: "Password"
                Layout.alignment: Qt.AlignRight
            }

            RowLayout {
                Layout.fillWidth: true

                Controls.TextField {
                    id: pwField
                    Layout.fillWidth: true

                    placeholderText: "Password of your email account"
                    echoMode: TextInput.Password
                }

                Controls.CheckBox {
                    text: "Show Password"
                    onClicked: {
                        if(pwField.echoMode == TextInput.Password) {
                            pwField.echoMode = TextInput.Normal;
                        } else {
                            pwField.echoMode = TextInput.Password;
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            Kirigami.Label {
                text: ""
            }

            Kirigami.Label {
                text: ""
            }

            Item {
                Layout.fillWidth: true

                Controls.Button {
                    text: "Discard"

                    onClicked: {
                        popup.close()
                    }
                }

                Controls.Button {
                    anchors.right: parent.right

                    text: "Save"

                    onClicked: {
                        popup.close()
                    }
                }
            }
        }
    }
}
