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
import Qt.labs.settings 1.0
import org.kube.framework 1.0 as Kube
import org.kube.extensionapi 1.0

Row {
    id: root
    property variant secret: null

    Component.onCompleted: {
        loadSecret(context.accountId)
    }

    function loadSecret(accountId) {
        if (settings.enabled) {
            ExtensionApi.loadSecret(accountId)
        }
    }

    function storeSecret(accountId, keyLookupName, secret) {
        if (settings.enabled) {
            ExtensionApi.storeSecret(accountId, keyLookupName, secret)
        }
    }

    Connections {
        target: ExtensionApi
        function onSecretAvailable(secret) {
            root.secret = secret
        }
    }

    spacing: Kube.Units.smallSpacing
    Kube.CheckBox {
        id: checkBox
        activeFocusOnTab: false
        checked: false
    }
    Kube.Label {
        text: qsTr("Enable GPG Keyring Integration")
        color: Kube.Colors.disabledTextColor
    }

    Settings {
        id: settings
        category: "extension.gpg"
        property alias enabled: checkBox.checked
    }
}
