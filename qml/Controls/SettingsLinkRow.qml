pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons"

Item {
    id: root

    property string text
    property url url
    property bool showStars: false

    signal clicked()

    implicitWidth: 240
    implicitHeight: _row.implicitHeight + 2 * AppTheme.spacing10

    Accessible.role: Accessible.Link
    Accessible.name: root.text

    Rectangle {
        anchors.fill: parent
        radius: AppTheme.radiusMedium
        color: _area.pressed
               ? AppTheme.pressed
               : (_area.containsMouse ? AppTheme.hover : "transparent")
    }

    RowLayout {
        id: _row

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: AppTheme.spacing4
        anchors.rightMargin: AppTheme.spacing4
        spacing: AppTheme.spacing8

        Text {
            Layout.fillWidth: true
            text: root.text
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fs14
            font.weight: Font.Medium
        }

        Row {
            visible: root.showStars
            Layout.alignment: Qt.AlignVCenter
            spacing: AppTheme.spacing2

            Repeater {
                model: 5

                Image {
                    width: 14
                    height: 14
                    source: "qrc:/assets/star.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }
        }

        Canvas {
            id: _chevron

            Layout.preferredWidth: 9
            Layout.preferredHeight: 14
            Layout.alignment: Qt.AlignVCenter

            onPaint: {
                const ctx = getContext("2d")
                ctx.reset()
                ctx.strokeStyle = AppTheme.textSecondary
                ctx.lineWidth = 2
                ctx.lineCap = "round"
                ctx.lineJoin = "round"
                ctx.beginPath()
                ctx.moveTo(1, 1)
                ctx.lineTo(8, 7)
                ctx.lineTo(1, 13)
                ctx.stroke()
            }

            Connections {
                target: AppTheme
                function onDarkModeChanged() { _chevron.requestPaint() }
            }
        }
    }

    MouseArea {
        id: _area

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onClicked: {
            if (root.url.toString().length > 0)
                Qt.openUrlExternally(root.url)
            root.clicked()
        }
    }
}
