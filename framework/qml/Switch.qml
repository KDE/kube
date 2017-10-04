import QtQuick 2.8
import QtQuick.Templates 2.2 as T
import org.kube.framework 1.0 as Kube


T.Switch {
    id: root

    implicitWidth: indicator.width
    implicitHeight: indicator.height

    indicator: Item {
        height: Kube.Units.gridUnit
        width: Kube.Units.gridUnit * 2

        Rectangle {
            width: parent.width
            height: parent.height
            color: root.checked ? Kube.Colors.highlightColor : Kube.Colors.buttonColor
            radius: 2
        }

        Rectangle {
            height: parent.height
            width: height
            x: root.visualPosition * Kube.Units.gridUnit
            radius: 2
            color: Kube.Colors.viewBackgroundColor
            border.width: 1
            border.color: Kube.Colors.buttonColor
        }
    }
}
