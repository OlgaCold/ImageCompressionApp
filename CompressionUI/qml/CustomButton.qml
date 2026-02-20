import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

Button {
    id: root

    scale: pressed ? 0.96 : 1.0
    Behavior on scale { NumberAnimation { duration: 100 } }

    background: Rectangle {
        color: root.pressed ? "#1e3d6d" : "#2b579a"
        radius: 4
        Behavior on color { ColorAnimation { duration: 100 } }
    }
    contentItem: Text {
        text: root.text
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.centerIn: parent
    }
    font.pixelSize: 14
    implicitWidth: 90
    implicitHeight: 28
}
