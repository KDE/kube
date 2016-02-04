/*
 * Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
//import org.kde.kube.mail 1.0 as Mail
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import org.kde.kube.mail 1.0 as Mail

Item {
    id: root

    function send() {
        composer.send()
    }

    function saveAsDraft() {
        composer.saveAsDraft()
    }

    function clear() {
        composer.clear();
    }

    Mail.Composer {
        id: composer
    }

    ColumnLayout {

        anchors.fill: parent

        GridLayout {

            columns: 2

            Label {
                text: "From"
            }

            ComboBox {
                model: composer.identityModel

                Layout.fillWidth: true

                currentIndex: composer.fromIndex

                onCurrentIndexChanged: {
                    composer.fromIndex = currentIndex
                }
            }

            Label {
                text: "To"
            }

            TextField {
                id: to

                Layout.fillWidth: true

                text: composer.to

                onTextChanged: {
                        composer.to = text;
                }
            }

            Label {
                text: "Cc"
            }

            TextField {
                id: cc

                Layout.fillWidth: true

                text: composer.cc

                onTextChanged: {
                        composer.cc = text;
                }
            }

            Label {
                text: "Bcc"
            }

            TextField {
                id: bcc

                Layout.fillWidth: true

                text: composer.bcc

                onTextChanged: {
                        composer.bcc = text;
                }
            }
        }

        TextField {
            id: subject

            Layout.fillWidth: true

            placeholderText: "Enter Subject"

            text: composer.subject

            onTextChanged: {
                composer.subject = text;
            }
        }

        TextArea {
            id: content

            text: composer.body

            onTextChanged: {
                composer.body = text;
            }

            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
