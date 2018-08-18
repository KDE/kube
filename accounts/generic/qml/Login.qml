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
import org.kube.accounts.generic 1.0 as GenericAccount

Item {
    property alias accountId: settings.accountIdentifier
    property string heading: qsTr("Login")
    property string subheadline: settings.accountName
    property bool valid: pwField.acceptableInput

    GenericAccount.Settings {
        id: settings
        accountType: "generic"
    }

    function login(){
        settings.login({accountSecret: pwField.text})
    }

    GridLayout {
        anchors {
            fill: parent
        }
        columns: 2
        columnSpacing: Kube.Units.largeSpacing
        rowSpacing: Kube.Units.largeSpacing

        Kube.Label {
            text: qsTr("Password")
            Layout.alignment: Qt.AlignRight
        }

        Kube.PasswordField {
            id: pwField
            Layout.fillWidth: true
            focus: true
            placeholderText: qsTr("Password of your account")
        }
    }
}
