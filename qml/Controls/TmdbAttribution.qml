pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons"

Item {
    id: root

    property bool compact: true
    property color textColor: AppTheme.textSecondary
    property real logoHeight: compact ? 16 : 26

    readonly property url logoSource: "qrc:/assets/tmdb_logo_blue.svg"
    readonly property real logoAspectRatio: 273.42 / 35.52

    signal opened()

    function openTmdbWebsite() {
        Qt.openUrlExternally("https://www.themoviedb.org")
        root.opened()
    }

    implicitWidth: _column.implicitWidth
    implicitHeight: _column.implicitHeight

    Accessible.role: Accessible.Link
    Accessible.name: qsTr("The Movie Database")
    Accessible.description: qsTr("Opens themoviedb.org")

    ColumnLayout {
        id: _column

        anchors.left: parent.left
        anchors.top: parent.top
        width: root.compact ? implicitWidth : parent.width
        spacing: AppTheme.spacing8

        Image {
            id: _logo

            Layout.preferredHeight: root.logoHeight
            Layout.preferredWidth: Layout.preferredHeight * root.logoAspectRatio
            source: root.logoSource
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            smooth: true
        }

        RowLayout {
            visible: !root.compact
            Layout.fillWidth: true
            spacing: AppTheme.spacing8

            Text {
                Layout.fillWidth: true
                text: qsTr("Thanks to TMDB for the movie and TV data.")
                color: root.textColor
                font.pixelSize: AppTheme.fs13
                font.weight: Font.DemiBold
                wrapMode: Text.Wrap
            }

            Canvas {
                id: _chevron

                Layout.preferredWidth: 9
                Layout.preferredHeight: 14
                Layout.alignment: Qt.AlignVCenter

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    ctx.strokeStyle = root.textColor
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
                    target: root
                    function onTextColorChanged() { _chevron.requestPaint() }
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        anchors.margins: -AppTheme.spacing8
        cursorShape: Qt.PointingHandCursor
        onClicked: root.openTmdbWebsite()
    }
}
