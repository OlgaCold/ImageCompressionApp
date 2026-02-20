import QtQuick 2.12
import QtQuick.Controls 2.12
import CompressionUI 1.0
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

    Component {
        id: errorComp;
        ErrorDialog {}
    }

    Loader {
        id: errorDialogLoader;
        anchors.fill: parent
        active: false
    }

    Connections {
        target: fm
        onErrorRequested: {
            errorDialogLoader.sourceComponent = errorComp
            errorDialogLoader.active = true
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

                CustomButton {
                    id: refreshBtn
                    text: qsTr("Refresh")
                    onClicked: fm.setDirectory(fm.directory)
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
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 0

                    ScrollBar.vertical: ScrollBar {
                        id: vScrollBar
                        policy: ScrollBar.AsNeeded
                        interactive: true
                        width: 12
                        contentItem: Rectangle {
                            color: "#cfcfcf"
                            radius: 6
                        }
                        visible: listView.contentHeight > listView.height
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

                            Item {
                                width: 380
                                height: parent.height
                                Text {
                                    id: fileNameText
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width
                                    text: name
                                    elide: Text.ElideMiddle
                                    wrapMode: Text.NoWrap
                                    color: "#111"
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true

                                    onEntered: {
                                        if (fileNameText.truncated) {
                                            let windowPos = fileNameText.mapToItem(null, 0, fileNameText.height + 8)
                                            customTooltip.show(
                                                windowPos.x,
                                                windowPos.y,
                                                fileNameText.text
                                            )
                                        }
                                    }

                                    onExited: customTooltip.close()
                                }
                            }

                            Text {
                                id: sizeOfFile;
                                text: (size/1024).toFixed(1) + " KB";
                                width: 120;
                                color:"#444"
                            }

                            CustomButton {
                                id: processBtn
                                text: qsTr("Process")
                                onClicked: fm.activate(index)
                            }

                            Text {
                                id: processingStatus;
                                text: status;
                                elide: Text.ElideRight;
                                width: 150;
                                color:"#111"
                            }
                        }
                    }
                }
            }
        }
    }

    CustomTooltip {
        id: customTooltip
    }
}
