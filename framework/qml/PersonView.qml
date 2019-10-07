 /*
  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
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

import QtQuick 2.7
import QtQuick.Controls 2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0 as Dialogs


import org.kube.framework 1.0 as Kube


Item {
    id: root

    property var controller: null

    ColumnLayout {
        RowLayout {
            spacing: Kube.Units.smallSpacing

            Kube.TextField {
                width: Kube.Units.gridUnit * 15
                placeholderText: qsTr("First name")
                backgroundColor: "white"
                text: controller.firstName
                onTextChanged: controller.firstName = text
            }

            Kube.TextField {
                width: Kube.Units.gridUnit * 15
                placeholderText: qsTr("Last name")
                backgroundColor: "white"
                text: controller.lastName
                onTextChanged: controller.lastName = text
            }
        }

        RowLayout {
            spacing: Kube.Units.largeSpacing
            Kube.ComboBox {
                width: parent.width

                model: Kube.EntityModel {
                    id: addressbookModel
                    type: "addressbook"
                    roles: ["name"]
                }
                textRole: "name"
                onCurrentIndexChanged: {
                    if (currentIndex >= 0) {
                        controller.addressbook = addressbookModel.data(currentIndex).object
                    }
                }
            }
            Kube.PositiveButton {
                id: createButton
                text: qsTr("Create Contact")
                onClicked: {
                    controller.saveAction.execute()
                    root.done()
                }
            }
        }
    }
}
