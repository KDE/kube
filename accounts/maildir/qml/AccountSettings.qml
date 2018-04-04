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
import QtQuick.Dialogs 1.0 as Dialogs
import org.kube.framework 1.0 as Kube
import org.kube.accounts.maildir 1.0 as MaildirAccount

Item {
    property string accountId
    property string heading: qsTr("Add your Maildir archive")
    property string subheadline: qsTr("To let Kube access your maildir archive, add the path to your archive and give the account a title that will be displayed inside Kube.")
    property bool valid: true
    implicitHeight: grid.implicitHeight

    MaildirAccount.MaildirSettings {
        id: maildirSettings
        accountIdentifier: accountId
        accountType: "maildir"
    }

    function save(){
        maildirSettings.save()
    }

    function remove(){
        maildirSettings.remove()
    }

    GridLayout {
        id: grid
        anchors.fill: parent
        columns: 2
        columnSpacing: Kube.Units.largeSpacing
        rowSpacing: Kube.Units.largeSpacing

        Kube.Label {
            text: qsTr("Title of Account")
            Layout.alignment: Qt.AlignRight
        }
        Kube.TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("E.g. \"Work\", \"Home\" that will be displayed in Kube as name")
            text: maildirSettings.accountName
            onTextChanged: {
                maildirSettings.accountName = text
            }
        }

        Kube.Label {
            text: qsTr("Path")
            Layout.alignment: Qt.AlignRight
        }
        RowLayout {
            Layout.fillWidth: true

            Kube.TextField {
                id: path
                Layout.fillWidth: true
                enabled: false
                text: maildirSettings.path
            }

            Kube.Button {
                iconName: Kube.Icons.folder

                onClicked: {
                    fileDialogComponent.createObject(parent)
                }

                Component {
                    id: fileDialogComponent
                    Dialogs.FileDialog {
                        id: fileDialog

                        visible: true
                        title: "Choose the maildir folder"

                        selectFolder: true

                        onAccepted: {
                            maildirSettings.path = fileDialog.fileUrl
                        }
                    }
                }
            }
        }
    }
}
