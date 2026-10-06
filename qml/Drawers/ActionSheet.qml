pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import "../Controls"
import "../Singletons"

Basic.Drawer {
    id: root

    property string heading
    property string subheading
    property var actions: []

    function openWith(newHeading, newSubheading, newActions) {
        root.heading = newHeading
        root.subheading = newSubheading
        root.actions = newActions
        root.open()
    }

    edge: Qt.BottomEdge
    implicitWidth: 420
    width: parent ? parent.width : implicitWidth
    height: parent
            ? Math.min(parent.height * 0.8, _content.implicitHeight + AppTheme.spacing20)
            : _content.implicitHeight
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

    contentItem: Flickable {
        contentWidth: width
        contentHeight: _content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: _content

            width: parent.width
            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: AppTheme.spacing20

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: AppTheme.spacing8
                    width: 36
                    height: 4
                    radius: 2
                    color: AppTheme.outlineStrong
                }
            }

            Text {
                Layout.fillWidth: true
                Layout.leftMargin: AppTheme.spacing18
                Layout.rightMargin: AppTheme.spacing18
                Layout.topMargin: AppTheme.spacing8
                text: root.heading
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fs18
                font.weight: Font.Bold
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                Layout.leftMargin: AppTheme.spacing18
                Layout.rightMargin: AppTheme.spacing18
                Layout.topMargin: AppTheme.spacing4
                visible: root.subheading.length > 0
                text: root.subheading
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fs13
                wrapMode: Text.Wrap
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: AppTheme.spacing12
            }

            Repeater {
                model: root.actions

                delegate: Rectangle {
                    id: _row

                    required property var modelData
                    required property int index

                    readonly property bool destructive: modelData.destructive === true
                    readonly property bool selected: modelData.selected === true

                    Layout.fillWidth: true
                    Layout.preferredHeight: AppTheme.touchTargetMinimum + AppTheme.spacing4
                    color: _rowArea.pressed ? AppTheme.pressed : "transparent"

                    Accessible.role: Accessible.Button
                    Accessible.name: modelData.label
                    Accessible.checked: selected

                    Rectangle {
                        visible: _row.index > 0
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.leftMargin: AppTheme.spacing18
                        anchors.rightMargin: AppTheme.spacing18
                        height: 1
                        color: AppTheme.outline
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: AppTheme.spacing18
                        anchors.rightMargin: AppTheme.spacing18
                        spacing: AppTheme.spacing12

                        Text {
                            Layout.fillWidth: true
                            text: _row.modelData.label
                            color: _row.destructive
                                   ? AppTheme.error
                                   : (_row.selected ? AppTheme.primary : AppTheme.textPrimary)
                            font.pixelSize: AppTheme.fs15
                            font.weight: _row.selected ? Font.Bold : Font.Medium
                            elide: Text.ElideRight
                        }

                        ThemedIcon {
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            visible: _row.selected
                            source: "qrc:/assets/check.png"
                            showPlaceholder: false
                            tintColor: AppTheme.primary
                        }
                    }

                    MouseArea {
                        id: _rowArea

                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            const handler = _row.modelData.handler
                            root.close()
                            if (handler)
                                handler()
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: AppTheme.spacing12
            }
        }
    }
}
