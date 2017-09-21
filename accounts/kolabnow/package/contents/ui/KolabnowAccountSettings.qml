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
    property string heading: qsTr("Connect your Kolab Now account")
    property string subheadline: qsTr("Please fill in your name and email address.")
    property bool valid: nameField.acceptableInput && emailField.acceptableInput

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

    GridLayout {
        anchors {
            fill: parent
        }
        columns: 2
        columnSpacing: Kube.Units.largeSpacing
        rowSpacing: Kube.Units.largeSpacing

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
                kolabnowSettings.accountName = text
            }
            placeholderText: qsTr("Your email address")
        }
    }
}
