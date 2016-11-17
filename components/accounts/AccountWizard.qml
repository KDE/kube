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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Controls 2.0 as Controls2
import org.kde.kirigami 1.0 as Kirigami

Controls2.Popup {
    id: popup

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    clip: true

    Controls2.StackView {
        id: stack

        anchors.fill: parent

        initialItem: mainView
    }

    Component {
        id: mainView

        Item {

            ColumnLayout {

                anchors.centerIn: parent

                width: parent.width * 0.4

                spacing: Kirigami.Units.largeSpacing

                Controls.Button {

                    Layout.fillWidth: true

                    text: "kolabnow account"

                    onClicked: {
                        stack.push(kolabnow)
                    }
                }

                Controls.Button {

                    Layout.fillWidth: true

                    text: "imap account"

                    onClicked: {
                        stack.push(imap)
                    }
                }

                Controls.Button {

                    Layout.fillWidth: true

                    text: "maildir archive"

                    onClicked: {
                        stack.push(maildir)
                    }
                }
            }
        }
    }

    Component {
        id: kolabnow

        CreateKolabNow {
        }
    }

    Component {
        id: imap

        CreateImap {
        }
    }

    Component {
        id: maildir

        CreateMaildir {
        }
    }
}
