/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

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
    property bool enablePartTreeView : true;
    property int desiredHeight: enablePartTreeView ? topPartLoader.height + newMailViewer.height + mailStructure.height + 50 : topPartLoader.height + newMailViewer.height + 50;

    Rectangle {
        id: rootRectangle
        anchors.fill: parent
        anchors.margins: 0
        ScrollView {
            id: scrollView
            anchors.fill: parent
            anchors.margins: 0
            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            Column {
                spacing: 2
                Text {
                    text: "New Mailviewer:"
                    color: "blue"
                }
                MV.MailViewer {
                    id: newMailViewer
                    debug: false
                    width: rootRectangle.width
                }
                Text {
                    text: "Old Mailviewer:"
                    color: "blue"
                }
                MessagePartTree {
                    id: topPartLoader
                    // width: rootRectangle.width
                    height: topPartLoader.contentHeight
                    width: topPartLoader.contentWidth >= rootRectangle.width ? topPartLoader.contentWidth : rootRectangle.width
                }
                TreeView {
                    id: mailStructure
                    visible: enablePartTreeView
                    width: rootRectangle.width
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
                    //model: messageParser.partTree
                    model: messageParser.newTree
                }
            }
       }
    }

    KubeFramework.MessageParser {
        id: messageParser
        message: root.message
    }
    html: messageParser.html
}
