
import QtQuick 2.7
import QtQuick.Layouts 1.2

import org.kube.components.theme 1.0 as KubeTheme

Item {
    height: 200
    width: 200

    RowLayout {

        anchors.fill: parent

        Rectangle {

            height: 50
            width: 50

            color: KubeTheme.Colors.text
        }
    }
}
