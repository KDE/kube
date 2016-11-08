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
import QtQuick.Controls 1.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import org.kube.framework.domain 1.0 as KubeFramework
import org.kube.mailviewer 1.0 as MV

Item {
    id: root
    property variant message;
    property string html;
    property int desiredHeight: topPartLoader.height + newMailViewer.height + attachments.height + 20

    clip: true

    MV.MailViewer {
        id: newMailViewer
        debug: false
        width: parent.width
    }

    //BEGIN old mail viewer

    MessagePartTree {
        id: topPartLoader
        anchors.top: newMailViewer.bottom

        Text {
            text: "old mailviewer"
            color: "blue"
        }

        visible: false
        // width: parent.width
        height: topPartLoader.contentHeight
        width: topPartLoader.contentWidth >= parent.width ? topPartLoader.contentWidth : parent.width
    }

    //END old mail viewer

    TreeView {
        id: attachments
        anchors {
            top: newMailViewer.bottom
            topMargin: 20
        }
        //visible: messageParser.attachments.rowCount() > 0
        width: parent.width
        height: 200
        TableViewColumn {
            role: "name"
            title: "Filename"
            width: 300
        }
        TableViewColumn {
            role: "type"
            title: "Type"
            width: 60
        }
        TableViewColumn {
            role: "size"
            title: "Size"
            width: 60
        }
        TableViewColumn {
            role: "encrypted"
            title: "Encrypted"
            width: 60
        }
        TableViewColumn {
            role: "signed"
            title: "Signed"
            width: 60
        }
        model: messageParser.attachments
    }

    TreeView {
        id: mailStructure
        visible: false
        anchors.top: messageParser.attachments.rowCount() > 0 ? attachments.bottom : newMailViewer.bottom
        width: parent.width
        height: 400
        TableViewColumn {
            role: "type"
            title: "Type"
            width: 300
        }
        TableViewColumn {
            role: "embeded"
            title: "Embeded"
            width: 60
        }
        TableViewColumn {
            role: "securityLevel"
            title: "SecurityLevel"
            width: 60
        }
        TableViewColumn {
            role: "content"
            title: "Content"
            width: 200
        }
        model: messageParser.newTree
    }

    KubeFramework.MessageParser {
        id: messageParser
        message: root.message
    }
    html: messageParser.html

}
