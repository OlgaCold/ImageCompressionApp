import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12

Dialog {
    id: dlg
    modal: true
    property alias message: messageText.text
    width: parent.width / 3

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    contentItem: Rectangle {
        id: container
        color: "#ffffff"
        radius: 10
        border.color: "#d0d0d0"
        border.width: 1

        width: parent.width
        implicitHeight: column.implicitHeight + 32

        Column {
            id: column
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Text {
                text: qsTr("Error")
                font.bold: true
                font.pixelSize: 18
                color: "#d32f2f"
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
            }

            Rectangle {
                height: 1
                color: "#eee"
                width: parent.width
            }

            Text {
                id: messageText
                text: "Unknown error"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 15
                color: "#333333"
                width: parent.width
            }

            CustomButton {
                id: okBtn
                text: qsTr("OK")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: dlg.close()
            }
        }
    }
}
