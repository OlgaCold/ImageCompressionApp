import QtQuick 2.12
import QtQuick.Controls 2.12
import MyQmlComponents 1.0
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

ApplicationWindow {
    id: win
    width: 800; height: 600
    minimumWidth: width; maximumWidth: width
    minimumHeight: height; maximumHeight: height
    visible: true
    title: qsTr("ImageCompressionApp")
    Material.theme: Material.Light
    Material.accent: Material.Blue

    property var fm: fileModel

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

        Rectangle { // header
            id: header
            color: "#f3f3f3"
            radius: 4
            Layout.fillWidth: true
            height: 48
            border.color: "#d0d0d0"
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                Text {
                    text: "Directory:"
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                }
                Text {
                    text: fm.directory
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    color: "#333"
                }
                Button {
                    onClicked: fm.setDirectory(fm.directory) // triggers rescan if implemented
                    background: Rectangle {
                        color: '#2d6bc7'
                        radius: 4
                    }
                    contentItem: Text {
                        text: qsTr("Refresh")
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.centerIn: parent
                    }
                    font.pixelSize: 14
                    implicitWidth: 90
                    implicitHeight: 28
                }
            }
        }

        Rectangle { // content frame
            color: "#ffffff"
            radius: 4
            border.color: "#d0d0d0"
            Layout.fillWidth: true
            Layout.fillHeight: true
            anchors.margins: 0
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                ListView {
                    id: listView
                    model: fm
                    clip: true
                    //anchors.fill: parent
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 0

                    // ensure scrollbar appears for long lists
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOn
                        interactive: true
                        width: 12
                        contentItem: Rectangle { color: "#cfcfcf"; radius: 6 }
                    }

                    delegate: Rectangle {
                        width: listView.width
                        height: 48
                        color: index % 2 ? '#ffffff' : '#a0a0a0'
                        border.color: '#2b2a2a'

                        Row {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12

                            Text { text: name; elide: Text.ElideRight; width: 380; color:"#111" }
                            Text { text: (size/1024).toFixed(1) + " KB"; width: 120; color:"#444" }
                            Button {
                                id: procBtn
                                text: qsTr("Process")
                                onClicked: fm.activate(index)
                                background: Rectangle {
                                    color: "#2b579a"
                                    radius: 4
                                }
                                contentItem: Text {
                                    text: procBtn.text
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    anchors.centerIn: parent
                                }
                                font.pixelSize: 14
                                implicitWidth: 90
                                implicitHeight: 28
                            }
                        }
                    }
                }
            }
        }
    }
}