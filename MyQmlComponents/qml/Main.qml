
import QtQuick 2.12
import QtQuick.Controls 2.12
import MyQmlComponents 1.0
import QtQuick.Layouts 1.12

ApplicationWindow {
    id: win
    width: 800; height: 600
    visible: true
    title: qsTr("ImageCompressionApp")

    FileModel {
        id: fm
        directory: startDirectory
    }

    Component { id: errorComp; ErrorDialog {} }
    Loader { id: errorDialogLoader; active: false }

    Connections {
        target: fm
        onErrorRequested: {
            errorDialogLoader.setSourceComponent(errorComp)
            errorDialogLoader.item.message = message
            errorDialogLoader.item.open()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Text {
            text: "Directory: " + fm.directory
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: fm

            delegate: Rectangle {
                width: listView.width
                height: 48
                color: index % 2 ? '#794d4d' : '#ce9f9f'

                Row {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 12

                    Text { text: name; elide: Text.ElideRight; width: 380 }
                    Text { text: (size/1024) + " KB"; width: 100 }
                    Text { text: status; color: status=="" ? "gray" : "blue"; width: 150 }
                    Button { text: qsTr("Process"); onClicked: fm.activate(index) }
                }
            }
        }
    }
}