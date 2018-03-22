/*
 *  Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


import QtQuick 2.7
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Column {
    id: root

    property variant controller

    spacing: Kube.Units.smallSpacing

    ListView {
        id: emails

        width: personComposerRoot.width - Kube.Units.largeSpacing
        height: contentHeight

        model: controller.model

        delegate: Row {
            height: textField.height + Kube.Units.smallSpacing
            spacing: Kube.Units.smallSpacing
            Kube.Label { text: qsTr("(main)") }
            Kube.TextField {id: textField; width: Kube.Units.gridUnit * 15; text: model.email; backgroundColor: "white" }
            Kube.IconButton {
                id: removeButton
                iconName: Kube.Icons.listRemove
                onClicked: root.controller.remove(model.id)
            }
        }
    }

    Kube.Button {
        id: button
        text: qsTr("Add")
        focus: true
        onClicked: {
            root.controller.add({email: ""});
        }
    }
}
