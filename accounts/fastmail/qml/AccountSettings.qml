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

Item {
    property alias accountId: fastmailSettings.accountIdentifier
    property string heading: qsTr("Connect your FastMail account")
    property string subheadline: qsTr("Please fill in your name and email address. Please note that you require an app-specific password and cannot use your regular FastMail password.")
    property bool valid: nameField.acceptableInput && emailField.acceptableInput
    implicitHeight: grid.implicitHeight

    FastmailSettings {
        id: fastmailSettings
    }

    function save(){
        fastmailSettings.save()
    }

    function remove(){
        fastmailSettings.remove()
    }

    GridLayout {
        id: grid
        anchors.fill: parent
        columns: 2
        columnSpacing: Kube.Units.largeSpacing
        rowSpacing: Kube.Units.largeSpacing

        Kube.Label {
            text: qsTr("Name")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            id: nameField
            objectName: "nameTextField"
            focus: true
            Layout.fillWidth: true
            placeholderText: qsTr("Your name")
            text: fastmailSettings.userName
            onTextChanged: {
                fastmailSettings.userName = text
            }
        }

        Kube.Label {
            text: qsTr("Email address")
            Layout.alignment: Qt.AlignRight
        }
        Kube.RequiredTextField {
            id: emailField
            objectName: "emailTextField"
            Layout.fillWidth: true

            text: fastmailSettings.emailAddress
            onTextChanged: {
                fastmailSettings.emailAddress = text
                fastmailSettings.accountName = text
            }
            placeholderText: qsTr("Your email address")
        }
    }
}
