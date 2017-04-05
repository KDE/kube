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

import QtQuick 2.4
import QtQuick.Controls 1.4 as Controls1
import QtQuick.Layouts 1.1

import org.kube.components.mailviewer 1.0 as MV
import org.kube.framework 1.0 as Kube

Item {
    id: root
    property variant message;
    property string html;
    property int desiredHeight: mailViewer.height + 20
    property variant attachments

    clip: true

    MV.MailViewer {
        id: mailViewer
        debug: false
        width: parent.width
    }

    Controls1.TreeView {
        id: mailStructure
        anchors.top: messageParser.attachments.rowCount() > 0 ? attachments.bottom : mailViewer.bottom
        visible: mailViewer.debug
        width: parent.width
        height: 400
        Controls1.TableViewColumn {
            role: "type"
            title: "Type"
            width: 300
        }
        Controls1.TableViewColumn {
            role: "embeded"
            title: "Embeded"
            width: 60
        }
        Controls1.TableViewColumn {
            role: "securityLevel"
            title: "SecurityLevel"
            width: 60
        }
        Controls1.TableViewColumn {
            role: "content"
            title: "Content"
            width: 200
        }
        model: messageParser.newTree
    }

    Kube.MessageParser {
        id: messageParser
        message: root.message
    }
    attachments: messageParser.attachments
    html: messageParser.html
}
