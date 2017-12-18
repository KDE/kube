/*
 * Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 * Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
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
import org.kube.framework 1.0 as Kube


FocusScope {
    id: root
    property string accountType
    signal done()

    property bool isFirstView: root.Controls2.StackView.index == 0
    property bool requireSetup: false

    function save() {
        if (loader.item.valid) {
            loader.item.save()
            Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": loader.item.accountIdentifier});
            root.done()
        } else {
            console.warn("Invalid settings.");
        }
    }

    //accountType -> uiPath
    Kube.AccountFactory {
        id: accountFactory
        accountType: root.accountType
    }

    Controls.ToolButton {
        id: backButton
        iconName: Kube.Icons.goBack
        tooltip: "go back"
        visible: !root.isFirstView
        onClicked: {
            stack.pop()
        }
    }

    Keys.onReturnPressed: save()

    //Item to avoid anchors conflict with stack
    Item {
        anchors{
            top: backButton.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        Kube.Heading {
            id: heading
            text: loader.item.heading
            color: Kube.Colors.highlightColor
        }

        Kube.Label {
            id: subHeadline

            anchors {
                left: heading.left
                top: heading.bottom
            }

            width: parent.width
            text: loader.item.subheadline
            color: Kube.Colors.disabledTextColor
            wrapMode: Text.Wrap
        }

        Item {
            id: accountEdit
            anchors {
                top:subHeadline.bottom
                left: parent.left
                right: parent.right
                topMargin: Kube.Units.largeSpacing * 2
            }

            Loader {
                id: loader
                anchors.fill: parent
                focus: true
                source: accountFactory.uiPath
            }
        }

        Item {
            id: spacer
            Layout.fillHeight: true
            anchors {
                top:accountEdit.bottom
                bottom: footer.top
                left: parent.left
                right: parent.right
            }
        }

        //This is where we should place the account wizard ui
        Item {
            id: footer

            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                topMargin: Kube.Units.largeSpacing * 2
            }

            Kube.Button {
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                }
                visible: !root.requireSetup

                text: qsTr("Discard")
                onClicked: {
                    root.done()
                }
            }

            Kube.PositiveButton {
                anchors {
                    right: parent.right
                    bottom: parent.bottom
                }

                text: qsTr("Save")
                onClicked: save()
            }
        }
    }
}
