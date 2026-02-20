import QtQuick 2.12
import QtQuick.Controls 2.12

Popup {
    id: root
    property alias tooltipText: label.text
    modal: false
    focus: false
    closePolicy: Popup.NoAutoClose
    z: 999

    padding: 0
    background: Item {}

    width: contentItem.implicitWidth
    height: contentItem.implicitHeight

    contentItem: Rectangle {
        color: "#333"
        radius: 4
        implicitWidth: label.implicitWidth + 12
        implicitHeight: label.implicitHeight + 8

        Text {
            id: label
            text: ""
            color: "white"
            anchors.centerIn: parent
        }
    }

    function show(x, y, text) {
        root.x = x
        root.y = y
        tooltipText = text
        open()
    }
}
