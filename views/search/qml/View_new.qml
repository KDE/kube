/*
 * Copyright (C) 2018 Michael Bohlender, <bohlender@kolabsys.com>
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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2

import org.kube.framework 1.0 as Kube

StackView {
    id: stack

    anchors.fill: parent

    property var searchTerm: ""

    initialItem: searchBar

    Component {
        id: searchBar

        FocusScope {

            function search() {
                stack.searchTerm = searchField.text
                stack.push(searchResults)
            }

            Rectangle {
                anchors.fill: parent
                color: Kube.Colors.backgroundColor

                Row {
                    anchors.centerIn: parent

                    spacing: Kube.Units.smallSpacing

                    Kube.TextField {
                        id: searchField
                        width: Kube.Units.gridUnit * 30
                        focus: true
                        text: stack.searchTerm

                        Keys.onEscapePressed: stack.searchTerm = ""
                        onAccepted: search()
                    }

                    Kube.PositiveButton {
                        text: qsTr("Search")
                        enabled: searchField.text != ""

                        onClicked: {
                            search()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: searchResults

        ColumnLayout {
            anchors.fill: parent

            spacing: 0

            Rectangle {
                id: toolbar

                width: parent.width
                height: Kube.Units.gridUnit + Kube.Units.largeSpacing

                color: Kube.Colors.backgroundColor

                Kube.IconButton {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: Kube.Units.smallSpacing
                    }
                    iconName: Kube.Icons.goBack
                    onClicked: {
                        stack.searchTerm = ""
                        stack.pop()
                    }
                }

                Kube.TextField {
                    id: searchBar

                    anchors.centerIn: parent

                    width: Kube.Units.gridUnit * 30

                    text: stack.searchTerm
                    placeholderText: qsTr("Search...")
                }
            }

            Rectangle {
                id: resultsArea

                Layout.fillHeight: true
                Layout.fillWidth: true

                color:  Kube.Colors.viewBackgroundColor

                RowLayout {
                    anchors.fill: parent

                    Rectangle {
                        width: Kube.Units.gridUnit * 10
                        Layout.fillHeight: true
                        color: Kube.Colors.buttonColor
                    }

                    Kube.ListView {
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        model: 10

                        delegate: Item {
                            width: parent.width
                            height: Kube.Units.gridUnit * 5

                            Rectangle {
                                anchors.centerIn: parent

                                height: parent.height - Kube.Units.largeSpacing
                                width: parent.width - Kube.Units.largeSpacing * 2

                                border.width: 1
                                border.color: Kube.Colors.buttonColor

                                Rectangle {
                                    height: parent.height
                                    width: height
                                    color: Kube.Colors.buttonColor
                                }

                                Kube.AbstractButton {
                                    anchors.fill: parent

                                    color: "transparent"

                                    onClicked: {
                                        stack.push(viewResult)
                                    }
                                }
                            }
                        }
                    }
                }

            }
        }
    }

    Component {
        id: viewResult

        ColumnLayout {
            anchors.fill: parent

            spacing: 0

            Rectangle {
                id: toolbar

                width: parent.width
                height: Kube.Units.gridUnit + Kube.Units.largeSpacing

                color: Kube.Colors.backgroundColor

                Kube.IconButton {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: Kube.Units.smallSpacing
                    }
                    iconName: Kube.Icons.goBack
                    onClicked: stack.pop()
                }

                Row {
                    anchors.centerIn: parent

                    spacing: Kube.Units.largeSpacing

                    Kube.Button {
                        text: "prev"
                    }

                    Kube.Button {
                        text: "next"
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
