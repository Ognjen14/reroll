pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import "../Controls"
import "../Singletons"

Basic.Drawer {
    id: root

    property string heading
    property var sections: []

    edge: Qt.BottomEdge
    implicitWidth: 420
    implicitHeight: 520
    width: parent ? parent.width : implicitWidth
    height: parent
            ? Math.min(parent.height * 0.8, implicitHeight)
            : implicitHeight
    modal: true
    dim: true
    interactive: false
    closePolicy: Basic.Popup.CloseOnEscape
                 | Basic.Popup.CloseOnPressOutside

    onOpened: PopupRegistry.register(root)
    onClosed: PopupRegistry.unregister(root)

    background: Rectangle {
        color: AppTheme.surface
        radius: AppTheme.radiusSheet
        border.width: 1
        border.color: AppTheme.outline

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.radius
            color: parent.color
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 44

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: AppTheme.spacing8
                width: 36
                height: 4
                radius: 2
                color: AppTheme.outlineStrong
            }

            AppButton {
                objectName: "tutorialSheetCloseButton"

                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: AppTheme.spacing12
                anchors.topMargin: AppTheme.spacing4
                shape: AppButton.CircleShape
                shapeSize: 32
                imageSource: "qrc:/assets/reroll_page/x.png"
                imageSize: 14
                backgroundColor: AppTheme.surfaceVariant
                foregroundColor: AppTheme.textSecondary
                accessibleName: qsTr("Close")

                onClicked: root.close()
            }
        }

        Basic.ScrollView {
            id: _scrollView

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            Column {
                width: _scrollView.availableWidth
                spacing: AppTheme.spacing20

                Text {
                    x: AppTheme.spacing18
                    width: parent.width - 2 * AppTheme.spacing18
                    text: root.heading
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fs22
                    font.weight: Font.Black
                    wrapMode: Text.Wrap
                }

                Repeater {
                    model: root.sections

                    delegate: Column {
                        required property var modelData

                        x: AppTheme.spacing18
                        width: parent.width - 2 * AppTheme.spacing18
                        spacing: AppTheme.spacing4

                        Text {
                            width: parent.width
                            text: modelData.heading
                            color: AppTheme.textPrimary
                            font.pixelSize: AppTheme.fs14
                            font.weight: Font.Bold
                            wrapMode: Text.Wrap
                        }

                        Text {
                            width: parent.width
                            text: modelData.body
                            color: AppTheme.textSecondary
                            font.pixelSize: AppTheme.fs13
                            lineHeight: 1.35
                            wrapMode: Text.Wrap
                        }
                    }
                }

                AppButton {
                    x: AppTheme.spacing18
                    width: parent.width - 2 * AppTheme.spacing18
                    height: AppTheme.controlHeightLarge
                    text: qsTr("Got it")
                    accessibleName: text
                    contentRadius: AppTheme.radiusPill

                    onClicked: root.close()
                }

                Item {
                    width: 1
                    height: 20
                }
            }
        }
    }
}
