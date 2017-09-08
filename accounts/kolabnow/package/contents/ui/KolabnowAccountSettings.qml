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
import QtQuick.Layouts 1.1
import org.kube.framework 1.0 as Kube
import org.kube.accounts.kolabnow 1.0 as KolabnowAccount

Item {

    property string accountId
    property string heading: qsTr("Connect your KolabNOW account")
    property string subheadline: qsTr("To let Kube access your account, fill in email address, username, password and give the account a title that will be displayed inside Kube.")
    property bool valid: accountField.acceptableInput && nameField.acceptableInput && emailField.acceptableInput && pwField.acceptableInput

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
            columnSpacing: Kube.Units.largeSpacing
            rowSpacing: Kube.Units.largeSpacing

            Kube.Label {
                text: qsTr("Title of Account")
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
                id: accountField
                Layout.fillWidth: true
                placeholderText: qsTr("E.g. \"Work\", \"Home\" that will be displayed in Kube as name")
                text: kolabnowSettings.accountName
                onTextChanged: {
                    kolabnowSettings.accountName = text
                }
                validator: RegExpValidator { regExp: /.*\S.*/ }
            }

            Kube.Label {
                text: qsTr("Name")
                Layout.alignment: Qt.AlignRight
            }
            Kube.RequiredTextField {
                id: nameField
                Layout.fillWidth: true
                placeholderText: qsTr("Your name")
                text: kolabnowSettings.userName
                onTextChanged: {
                    kolabnowSettings.userName = text
                }
            }

            Kube.Label {
                text: qsTr("Email address")
                Layout.alignment: Qt.AlignRight
            }
            Kube.RequiredTextField {
                id: emailField
                Layout.fillWidth: true

                text: kolabnowSettings.emailAddress
                onTextChanged: {
                    kolabnowSettings.emailAddress = text
                }
                placeholderText: qsTr("Your email address")
            }

            Kube.Label {
                text: qsTr("Password")
                Layout.alignment: Qt.AlignRight
            }

            Kube.PasswordField {
                id: pwField
                Layout.fillWidth: true

                placeholderText: qsTr("Password of your email account")
                text: kolabnowSettings.imapPassword
                onTextChanged: {
                    kolabnowSettings.imapPassword = text
                    kolabnowSettings.smtpPassword = text
                }
            }
        }
    }
}
