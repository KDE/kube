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
import QtQuick.Controls 2.0 as Controls2
import org.kube.framework 1.0 as Kube

Kube.Popup {
    id: root

    property bool requireSetup: false
    property var availableAccountPlugins: []

    modal: true
    focus: true
    closePolicy: requireSetup ? Controls2.Popup.NoAutoClose : Controls2.Popup.CloseOnEscape | Controls2.Popup.CloseOnPressOutside

    clip: true

    Controls2.StackView {
        id: stack
        anchors.fill: parent
        Component.onCompleted: {
            //If we only have one account type we skip the selection
            if (root.availableAccountPlugins.length == 1) {
                stack.push(wizardPage.createObject(app, {accountType: root.availableAccountPlugins[0]}))
            } else {
                stack.push(mainView.createObject(app))
            }
        }
    }

    Component {
        id: mainView

        Item {
            Kube.Heading {
                id: heading
                text: qsTr("Select your new account type")
                color: Kube.Colors.highlightColor
            }

            ColumnLayout {
                anchors.centerIn: parent
                width: parent.width * 0.4

                spacing: Kube.Units.largeSpacing

                Repeater {
                    model: root.availableAccountPlugins
                    delegate: Kube.Button {
                        Layout.fillWidth: true
                        text: modelData
                        onClicked: {
                            stack.push(wizardPage.createObject(app, {accountType:modelData}))
                        }
                    }
                }
            }
        }
    }

    Component {
        id: wizardPage
        AccountWizardPage {
            requireSetup: root.requireSetup
            onDone: {
                root.close()
                Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
            }
        }
    }
}
