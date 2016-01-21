import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0

import org.kde.sink.settings 1.0 as Settings

Item {
    id: root

    height: 150
    width: 600

    Settings.Maildir {
        id: maildir
    }

    ColumnLayout {
        anchors {
            fill: parent
            margins: 10
        }

        GridLayout {
            id: mainContent

            Layout.fillWidth: true

            rows: 2
            columns: 3

            Label {
                text: "Name:"
            }

            TextField {

                Layout.fillWidth: true

                placeholderText: "Enter a name to identify the resource"
                text: maildir.name
            }

            Label {

            }

            Label {
                text: "Location:"
            }

            TextField {

                Layout.fillWidth: true

                placeholderText: "Click on the folder button to enter the location"
                text: maildir.folderUrl

                enabled: false
            }

            Button {
                iconName: "folder"

                onClicked:  {
                    fileDialog.visible = true
                }

                FileDialog {
                    id: fileDialog
                    title: "Please choose the maildir folder"
                    folder: shortcuts.home

                    selectFolder: true

                    onAccepted: {
                        maildir.folderUrl = fileDialog.fileUrl
                        visible: false
                    }
                    onRejected: {
                        visible: false
                    }
                }
            }
        }

        RowLayout {
            id: buttons

            Layout.alignment: Qt.AlignRight

            Button {
                text: "Create Resource"
            }
            Button {
                text: "Cancel"
            }
        }
    }
}