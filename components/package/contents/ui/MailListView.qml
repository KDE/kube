/*
 * Copyright (C) 2015 Michael Bohlender <michael.bohlender@kdemail.net>
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
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.1

import org.kde.kirigami 1.0 as Kirigami

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.framework.theme 1.0

Controls.ScrollView {
    id: root
    property variant parentFolder
    property variant currentMail

    ListView {
        id: listView

        model: KubeFramework.MailListModel {
            parentFolder: root.parentFolder
        }

        Keys.onDownPressed: {
            incrementCurrentIndex()
        }
        Keys.onUpPressed: {
            decrementCurrentIndex()
        }
        focus: true

        delegate: Kirigami.AbstractListItem {
            width: listView.width
            height: Unit.size * 12

            enabled: true
            supportsMouseEvents: true
            checked: listView.currentIndex == index

            onClicked:  {
                listView.currentIndex = model.index
                root.currentMail = model.domainObject
            }

            RowLayout {
                Avatar {
                    id: avatar

                    height: Unit.size * 9
                    width: height

                    name: model.senderName
                }

                ColumnLayout {

                    Kirigami.Label {
                        text: model.senderName

                        font.weight: Font.DemiBold
                    }

                    Kirigami.Label {
                        text: model.subject
                    }

                    Kirigami.Label {
                        text: Qt.formatDateTime(model.date)

                        font.weight: Font.Light
                    }
                }

            }
        }
    }
}
