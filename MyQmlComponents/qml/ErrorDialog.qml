import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog {
    id: dlg
    property alias message: messageText.text
    modal: true
    title: qsTr("Error")
    contentItem: Column {
        spacing: 12; padding: 12
        Text { id: messageText; text: "Unknown file"; wrapMode: Text.WordWrap }
        Button {
            id: okBtn
            text: qsTr("Ok")
            onClicked: { okBtn.background.color = "#d0ffd0"; dlg.close(); }
        }
    }
}