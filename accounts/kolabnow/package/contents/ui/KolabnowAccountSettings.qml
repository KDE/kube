/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

import QtQuick 2.4
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.1

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.settings 1.0 as KubeSettings
import org.kube.accounts.kolabnow 1.0 as KolabnowAccount

Item {

    property string accountId
    property string heading: "Connect your KolabNOW account"
    property string subheadline: "To let Kube access your account, fill in email address, username, password and give the account a title that will be displayed inside Kube."

    KolabnowAccount.KolabnowSettings {
        id: kolabnowSettings
        accountIdentifier: accountId
        accountType: "kolabnow"
    }

    function save(){
        kolabnowSettings.save()
    }

    function remove(){
        kolabnowSettings.remove()
    }

    Item {
        anchors {
            fill: parent
        }

        GridLayout {
            anchors {
                fill: parent
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
                text: kolabnowSettings.accountName
                onTextChanged: {
                    kolabnowSettings.accountName = text
                }
            }

            Controls.Label {
                text: "Name"
                Layout.alignment: Qt.AlignRight
            }
            Controls.TextField {
                Layout.fillWidth: true
                placeholderText: "Your name"
                text: kolabnowSettings.userName
                onTextChanged: {
                    kolabnowSettings.userName = text
                }
            }

            Controls.Label {
                text: "Email address"
                Layout.alignment: Qt.AlignRight
            }
            Controls.TextField {
                Layout.fillWidth: true

                text: kolabnowSettings.emailAddress
                onTextChanged: {
                    kolabnowSettings.emailAddress = text
                }
                placeholderText: "Your email address"
            }

            Controls.Label {
                text: "Password"
                Layout.alignment: Qt.AlignRight
            }
            RowLayout {
                Layout.fillWidth: true

                Controls.TextField {
                    id: pwField
                    Layout.fillWidth: true

                    placeholderText: "Password of your email account"
                    text: kolabnowSettings.imapPassword
                    onTextChanged: {
                        kolabnowSettings.imapPassword = text
                        kolabnowSettings.smtpPassword = text
                    }

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
        }
    }
}
