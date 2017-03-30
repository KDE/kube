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
import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.accounts 1.0 as KubeAccounts
import org.kube.components.theme 1.0 as KubeTheme

Item {
    id: root
    property string accountType

    KubeAccounts.AccountFactory {
        id: accountFactory
        accountType: root.accountType
    }

    Controls.ToolButton {
        iconName: "go-previous"
        tooltip: "go back"
        onClicked: {
            stack.pop()
        }
    }

    //Item to avoid anchors conflict with stack
    Item {

        anchors {
            fill: parent
            margins: Kirigami.Units.largeSpacing * 2
        }

        Kirigami.Heading {
            id: heading
            text: loader.item.heading
            color: KubeTheme.Colors.highlightColor
        }

        Kirigami.Label {
            id: subHeadline

            anchors {
                left: heading.left
                top: heading.bottom
            }

            width: parent.width
            text: loader.item.subheadline
            color: KubeTheme.Colors.disabledTextColor
            wrapMode: Text.Wrap
        }

        Item {
            id: accountEdit
            anchors {
                top:subHeadline.bottom
                left: parent.left
                right: parent.right
                topMargin: Kirigami.Units.largeSpacing * 2
            }

            Loader {
                id: loader
                anchors.fill: parent
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
        GridLayout {
            id: footer
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                topMargin: Kirigami.Units.largeSpacing * 2
            }

            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            Item {
                Layout.fillHeight: true
            }

            Kirigami.Label {
                text: ""
            }

            Item {
                Layout.fillWidth: true

                Controls.Button {
                    text: "Discard"

                    onClicked: {
                        popup.close()
                    }
                }

                Controls.Button {
                    anchors.right: parent.right
                    text: "Save"
                    onClicked: {
                        loader.item.save()
                        popup.close()
                    }
                }
            }
        }
    }
}
