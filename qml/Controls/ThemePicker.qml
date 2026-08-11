pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Singletons"
import com.topicdev.reroll 1.0

Item {
    id: root

    implicitHeight: _pill.implicitHeight

    readonly property string currentMode: AppSettings.themeMode || "light"
    readonly property var modes: [
        {
            key: "light",
            label: qsTr("Light"),
            icon: "qrc:/assets/sun_light_theme.svg"
        },
        {
            key: "dark",
            label: qsTr("Dark"),
            icon: "qrc:/assets/moon_dark_theme.svg"
        },
        {
            key: "system",
            label: qsTr("System"),
            icon: "qrc:/assets/system_theme.svg"
        }
    ]

    function applyMode(mode) {
        AppSettings.themeMode = mode
    }

    Rectangle {
        id: _pill

        anchors.left:        parent.left
        anchors.leftMargin:  16
        anchors.right:       parent.right
        anchors.rightMargin: 16
        implicitHeight: _row.implicitHeight + 8
        radius: height / 2
        color: AppTheme.surface

        Behavior on color {
            ColorAnimation { duration: 180 }
        }

        RowLayout {
            id: _row

            anchors.centerIn: parent
            spacing: 4

            Repeater {
                model: root.modes

                delegate: AbstractButton {
                    id: _option

                    required property var modelData

                    readonly property string modeKey: modelData.key
                    readonly property bool selected: root.currentMode === modeKey

                    implicitWidth: _optionRow.implicitWidth + leftPadding + rightPadding
                    implicitHeight: _optionRow.implicitHeight + topPadding + bottomPadding

                    leftPadding: 14
                    rightPadding: 16
                    topPadding: 9
                    bottomPadding: 9

                    hoverEnabled: true
                    focusPolicy: Qt.StrongFocus
                    scale: down ? 0.96 : 1

                    Accessible.name: modelData.label
                    Accessible.description: qsTr("Select %1 appearance").arg(modelData.label)
                    Accessible.role: Accessible.RadioButton
                    Accessible.checked: selected

                    onClicked: root.applyMode(modeKey)

                    Behavior on scale {
                        NumberAnimation {
                            duration: 110
                            easing.type: Easing.OutCubic
                        }
                    }

                    background: Rectangle {
                        radius: height / 2
                        color: _option.selected
                               ? AppTheme.surfaceRaised
                               : _option.down
                                 ? AppTheme.pressed
                                 : _option.hovered
                                   ? AppTheme.hover
                                   : "transparent"
                        border.width: _option.visualFocus ? 2 : 0
                        border.color: AppTheme.primary

                        Behavior on color {
                            ColorAnimation { duration: 150 }
                        }
                    }

                    contentItem: Item {
                        implicitWidth: _optionRow.implicitWidth
                        implicitHeight: _optionRow.implicitHeight

                        RowLayout {
                            id: _optionRow
                            anchors.centerIn: parent
                            spacing: 7

                            ThemedIcon {
                                Layout.preferredWidth: 18
                                Layout.preferredHeight: 18
                                source: _option.modelData.icon
                                tintColor: _option.selected ? AppTheme.textPrimary : AppTheme.textSecondary
                                placeholderBackground: "transparent"
                                placeholderBorderColor: AppTheme.textSecondary
                                placeholderTextColor: AppTheme.textSecondary
                            }

                            Text {
                                text: _option.modelData.label
                                color: _option.selected ? AppTheme.textPrimary : AppTheme.textSecondary
                                font.pixelSize: AppTheme.fs14
                                font.weight: _option.selected ? Font.DemiBold : Font.Medium

                                Behavior on color {
                                    ColorAnimation { duration: 150 }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}