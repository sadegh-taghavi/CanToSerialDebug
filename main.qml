import QtQuick 2.15
import QtQuick.Controls 2.15
import org.tdevelopers.serialdebug 1.0

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Scroll")


    Rectangle {
        id: btnSend
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 2
        width: 40
        height: txtField.height
        color: btnSendArea.pressed ? "gray" : "darkgray"
        Behavior on color {
            ColorAnimation {
                duration: 100
            }
        }

        border.color: "darkgray"
        border.width: 2
        radius: 3
        Text {
            text: qsTr("Send!")
            font.pointSize: 10
            color: "white"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.fill: parent
        }
        MouseArea {
            id: btnSendArea
            anchors.fill: parent
            onClicked: {
                mdlSerialDevice.sendString( txtField.text )
            }
        }
    }

    TextField {
        id: txtField

        anchors.margins: 4
        anchors.left: parent.left
        anchors.right: btnSend.left
        anchors.top: parent.top
        font.pointSize: 12
        text:"0x7df 0x02 0x01 0x0c 0x55 0x55 0x55 0x55 0x55"
    }

    ListView {
        id: listView
        anchors.top: txtField.bottom
        anchors.bottom: btnClearLog.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: SerialDevice{id: mdlSerialDevice}

        delegate: Item {
            width: listView.width
            height: 20

            Rectangle {
                anchors.fill: parent
                color: "red"
                opacity: 0.0
                NumberAnimation on opacity {
                    id: opacityAnimation
                    from: 0.3
                    to: 0.0
                    duration: 100
                    loops: 1
                    running: false
                }


            }

            Text {
                anchors.fill: parent
                anchors.margins: 2
                text: textData
                onTextChanged: {
                    if( !mouseArea.pressed )
                        opacityAnimation.start()
                }

                font.pointSize:  mouseArea.pressed ? 11 : 10
                font.bold: mouseArea.pressed
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
            }
        }
    }

    Rectangle {
        anchors.right: txtFlickable.right
        visible: txtFlickable.contentHeight > txtFlickable.height
        y: txtFlickable.y + txtFlickable.visibleArea.yPosition * txtFlickable.height
        width: 3
        radius: 2
        height: txtFlickable.visibleArea.heightRatio * txtFlickable.height
        color: "gray"
    }

    Rectangle{
        id: btnClearLog
        anchors.bottom: txtFlickable.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 20
        color: btnClearLogArea.pressed ? "darkgray" : "gray"
        Behavior on color {
            ColorAnimation {
                duration: 100
            }
        }
        Text {
            text: qsTr("Clear log!")
            font.pointSize: 10
            color: "white"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.fill: parent
        }
        MouseArea {
            id: btnClearLogArea
            anchors.fill: parent
            onClicked: {
                mdlSerialDevice.logText = ""
            }
        }
    }
    CheckBox {
        id: chkAutoScroll
        anchors.left: btnClearLog.left
        anchors.verticalCenter: btnClearLog.verticalCenter
        text: tr("Auto Scroll log")
        font.pointSize: 10
        checked: true
    }
    Flickable {
        id: txtFlickable
        height: 100
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4
        contentWidth: txtArea.contentWidth
        contentHeight: txtArea.contentHeight
        onContentHeightChanged: {
            if( chkAutoScroll.checked )
            {
                txtFlickable.contentY = txtFlickable.contentHeight - txtFlickable.height
            }
        }

        clip: true

        TextArea {
            id: txtArea
            readOnly: true
            text: mdlSerialDevice.logText
            font.pointSize: 10
            onTextChanged: {
                mdlSerialDevice.logText = text
            }
            background: Item {}
        }
    }

}
