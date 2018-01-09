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
import org.kube.framework 1.0 as Kube

Item {
    id: root
    property string accountId
    property bool canRemove: true

    signal done()

    function login() {
        loader.item.login()
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"accountId": loader.item.accountIdentifier});
        root.done()
    }

    Kube.AccountFactory {
        id: accountFactory
        accountId: root.accountId
    }

    Item {
        anchors {
            fill: parent
            margins: Kube.Units.largeSpacing * 2
        }

        Kube.Heading {
            id: heading
            text: loader.item ? loader.item.heading : ""
            color: Kube.Colors.highlightColor
        }

        Kube.Label {
            id: subHeadline

            anchors {
                left: heading.left
                top: heading.bottom
            }

            width: parent.width
            text: loader.item ? loader.item.subheadline : ""
            color: Kube.Colors.disabledTextColor
            wrapMode: Text.Wrap
        }

        Item {
            id: accountEdit
            anchors {
                top: subHeadline.bottom
                left: parent.left
                right: parent.right
                bottom: footer.top
                topMargin: Kube.Units.largeSpacing * 2
            }

            Loader {
                id: loader
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                //The initial size is somehow necessary so the loader is properly anchored
                height: 10
                source: accountFactory.loginUi
                focus: true
                onLoaded: item.accountId = root.accountId
            }
            Item {
                id: spacer
                anchors {
                    top: loader.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
            }
        }
        Keys.onReturnPressed: login()

        Item {
            id: footer
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                topMargin: Kube.Units.largeSpacing * 2
            }

            Kube.Button {
                anchors.right: parent.right
                text: qsTr("Login")
                onClicked: login()
            }
        }
    }
}
