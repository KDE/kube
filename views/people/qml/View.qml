/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
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
import QtQuick.Controls 2.1
import org.kube.framework 1.0 as Kube

Kube.View {

    onRefresh: {
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "addressbook"})
        Kube.Fabric.postMessage(Kube.Messages.synchronize, {"type": "contact"})
    }

    People {
        id: people
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: Kube.Units.smallSpacing
    }
}
