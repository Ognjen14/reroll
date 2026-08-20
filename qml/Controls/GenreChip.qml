pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import "../Singletons"

AbstractButton {
    id: root

    property int genreId: 0
    property bool selected: false
    property color selectedBackgroundColor: Qt.rgba(AppTheme.primary.r,
                                                    AppTheme.primary.g,
                                                    AppTheme.primary.b,
                                                    0.12)
    property color selectedForegroundColor: AppTheme.primary
    property color selectedBorderColor: AppTheme.primary
    property color unselectedBackgroundColor: AppTheme.surfaceVariant
    property color unselectedForegroundColor: AppTheme.textSecondary
    property color unselectedBorderColor: "transparent"
    property color disabledBackgroundColor: AppTheme.surfaceVariant
    property color disabledForegroundColor: AppTheme.textDisabled

    readonly property bool focusVisible: visualFocus
    readonly property color effectiveBackgroundColor: !enabled
                                                       ? disabledBackgroundColor
                                                       : (selected
                                                          ? selectedBackgroundColor
                                                          : unselectedBackgroundColor)
    readonly property color effectiveForegroundColor: !enabled
                                                       ? disabledForegroundColor
                                                       : (selected
                                                          ? selectedForegroundColor
                                                          : unselectedForegroundColor)

    signal selectionRequested(int genreId, bool selected)

    readonly property color effectiveBorderColor: !enabled
                                                   ? AppTheme.outline
                                                   : (selected
                                                      ? selectedBorderColor
                                                      : unselectedBorderColor)

    implicitWidth: Math.max(56, _label.implicitWidth + leftPadding + rightPadding)
    implicitHeight: AppTheme.controlHeightMedium
    leftPadding: AppTheme.spacing14
    rightPadding: AppTheme.spacing14
    topPadding: AppTheme.spacing8
    bottomPadding: AppTheme.spacing8

    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    Accessible.role: Accessible.CheckBox
    Accessible.name: text
    Accessible.checked: selected

    onClicked: selectionRequested(genreId, !selected)

    contentItem: Text {
        id: _label

        text: root.text
        color: root.effectiveForegroundColor
        font.pixelSize: AppTheme.fs12
        font.weight: root.selected ? Font.DemiBold : Font.Medium
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }

    background: Rectangle {
        radius: AppTheme.radiusLarge
        color: root.effectiveBackgroundColor
        border.width: root.focusVisible ? 2 : 1
        border.color: root.focusVisible
                      ? AppTheme.focus
                      : root.effectiveBorderColor

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: !root.selected
            color: root.down
                   ? AppTheme.pressed
                   : (root.hovered ? AppTheme.hover : "transparent")
        }

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }
}
