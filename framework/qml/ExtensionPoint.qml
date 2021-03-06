/*
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

import org.kube.framework 1.0 as Kube

Repeater {
    id: root
    property alias extensionPoint: extensionModel.extensionPoint
    property variant context: {}
    property Item item: null

    model: Kube.ExtensionModel {
        id: extensionModel
    }
    Loader {
        source: root.model.findSource(model.name, "main.qml")
        property variant context: root.context
        onLoaded: {
            root.item = item
        }
    }
}
