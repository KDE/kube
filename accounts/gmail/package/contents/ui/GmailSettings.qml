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
import org.kube.framework 1.0 as Kube
import org.kube.accounts.gmail 1.0 as GmailAccount

Item {

    property string accountId
    property string heading: "Connect your GMail account"
    property string subheadline: "To let Kube access your account, fill in email address, username, password and give the account a title that will be displayed inside Kube."

    GmailAccount.GmailSettings {
        id: gmailSettings
        accountIdentifier: accountId
        accountType: "gmail"
    }

    function save(){
        gmailSettings.save()
    }

    function remove(){
        gmailSettings.remove()
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
                text: "Please note that GMail requires you to configure your account to allow IMAP connections from Kube:
<ol type=''>
<li> See <a href='https://support.google.com/mail/answer/7126229'>https://support.google.com/mail/answer/7126229</a> to configure your account to allow IMAP connections.
<li> Visit <a href='https://myaccount.google.com/lesssecureapps'>https://myaccount.google.com/lesssecureapps</a> and enable the setting to allow Kube to connect to your account."
                Layout.alignment: Qt.AlignCenter
                Layout.columnSpan: 2
                onLinkActivated: Qt.openUrlExternally(link)
                textFormat: Text.StyledText
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            Kube.Label {
                text: qsTr("Title of Account")
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
                Layout.fillWidth: true
                placeholderText: "E.g. \"Work\", \"Home\" that will be displayed in Kube as name"
                text: gmailSettings.accountName
                onTextChanged: {
                    gmailSettings.accountName = text
                }
            }

            Kube.Label {
                text: qsTr("Name")
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
                Layout.fillWidth: true
                placeholderText: "Your name"
                text: gmailSettings.userName
                onTextChanged: {
                    gmailSettings.userName = text
                }
            }

            Kube.Label {
                text: qsTr("Email address")
                Layout.alignment: Qt.AlignRight
            }
            Kube.TextField {
                Layout.fillWidth: true

                text: gmailSettings.emailAddress
                onTextChanged: {
                    gmailSettings.emailAddress = text
                }
                placeholderText: "Your email address"
            }

            Kube.Label {
                text: qsTr("Password")
                Layout.alignment: Qt.AlignRight
            }
            RowLayout {
                Layout.fillWidth: true

                Kube.TextField {
                    id: pwField
                    Layout.fillWidth: true

                    placeholderText: "Password of your email account"
                    text: gmailSettings.imapPassword
                    onTextChanged: {
                        gmailSettings.imapPassword = text
                        gmailSettings.smtpPassword = text
                    }

                    echoMode: TextInput.Password
                }

                Controls.CheckBox {
                    text: qsTr("Show Password")
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
