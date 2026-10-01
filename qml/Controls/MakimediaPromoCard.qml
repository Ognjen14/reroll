pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons"

Item {
    id: root

    property url url: "https://makimedia.org/"
    readonly property color brandRed: "#F23636"

    signal clicked()

    implicitWidth: 320
    implicitHeight: _content.implicitHeight + 2 * AppTheme.spacing16

    Accessible.role: Accessible.Link
    Accessible.name: qsTr("Makimedia")
    Accessible.description: qsTr("Opens makimedia.org")

    Rectangle {
        id: _card

        anchors.fill: parent
        radius: AppTheme.radiusLarge
        color: AppTheme.surfaceVariant
        border.width: 1
        border.color: _area.containsMouse ? root.brandRed : AppTheme.outline
        clip: true
        scale: _area.pressed ? 0.98 : 1

        Behavior on scale { NumberAnimation { duration: 90 } }
        Behavior on border.color { ColorAnimation { duration: 120 } }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            opacity: AppTheme.darkMode ? 0.22 : 0.10
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: root.brandRed }
                GradientStop { position: 0.55; color: "transparent" }
            }
        }
    }

    RowLayout {
        id: _content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: AppTheme.spacing16
        anchors.rightMargin: AppTheme.spacing16
        spacing: AppTheme.spacing14

        Image {
            Layout.preferredWidth: 56
            Layout.preferredHeight: 56
            Layout.alignment: Qt.AlignVCenter
            source: "qrc:/assets/makimedia_icon.png"
            sourceSize.width: 112
            sourceSize.height: 112
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: AppTheme.spacing4

            RowLayout {
                spacing: AppTheme.spacing8

                Text {
                    text: qsTr("Makimedia")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fs16
                    font.weight: Font.Bold
                }

                Rectangle {
                    implicitWidth: _free.implicitWidth + 2 * AppTheme.spacing6
                    implicitHeight: _free.implicitHeight + AppTheme.spacing2
                    radius: AppTheme.radiusPill
                    color: Qt.rgba(0.95, 0.21, 0.21, 0.18)

                    Text {
                        id: _free

                        anchors.centerIn: parent
                        text: qsTr("FREE")
                        color: root.brandRed
                        font.pixelSize: AppTheme.fs10
                        font.weight: Font.Black
                        font.letterSpacing: 0.8
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Found something to watch? Play your own films and shows on your PC, phone, tablet and TV.")
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fs13
                wrapMode: Text.Wrap
            }

            Flow {
                Layout.fillWidth: true
                Layout.topMargin: AppTheme.spacing2
                spacing: AppTheme.spacing6

                Repeater {
                    model: [qsTr("Windows"), qsTr("Android"), qsTr("Android TV")]

                    Rectangle {
                        required property string modelData

                        implicitWidth: _platform.implicitWidth + 2 * AppTheme.spacing8
                        implicitHeight: _platform.implicitHeight + AppTheme.spacing4
                        radius: AppTheme.radiusPill
                        color: "transparent"
                        border.width: 1
                        border.color: AppTheme.outlineStrong

                        Text {
                            id: _platform

                            anchors.centerIn: parent
                            text: parent.modelData
                            color: AppTheme.textSecondary
                            font.pixelSize: AppTheme.fs11
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: _visit.implicitWidth + 2 * AppTheme.spacing14
            implicitHeight: 36
            radius: AppTheme.radiusPill
            color: root.brandRed

            Text {
                id: _visit

                anchors.centerIn: parent
                text: qsTr("Visit")
                color: "#FFFFFF"
                font.pixelSize: AppTheme.fs13
                font.weight: Font.Bold
            }
        }
    }

    MouseArea {
        id: _area

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onClicked: {
            Qt.openUrlExternally(root.url)
            root.clicked()
        }
    }
}
