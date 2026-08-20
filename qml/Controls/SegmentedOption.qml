pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import "../Singletons"

AbstractButton {
    id: root

    property bool selected: false
    property color selectedBackgroundColor: AppTheme.primary
    property color selectedForegroundColor: AppTheme.darkMode ? AppTheme.onPrimary : "#FFFFFF"
    property color unselectedBackgroundColor: "transparent"
    property color unselectedForegroundColor: AppTheme.textSecondary

    property real cornerRadius: AppTheme.radiusLarge

    implicitWidth: Math.max(72, _label.implicitWidth + leftPadding + rightPadding)
    implicitHeight: AppTheme.controlHeightMedium
    leftPadding: AppTheme.spacing12
    rightPadding: AppTheme.spacing12
    topPadding: AppTheme.spacing8
    bottomPadding: AppTheme.spacing8

    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    Accessible.role: Accessible.RadioButton
    Accessible.name: text
    Accessible.checked: selected

    contentItem: Text {
        id: _label

        text: root.text
        color: root.selected
               ? root.selectedForegroundColor
               : root.unselectedForegroundColor
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
        radius: root.cornerRadius
        color: root.selected
               ? root.selectedBackgroundColor
               : root.unselectedBackgroundColor
        border.width: root.visualFocus ? 2 : 0
        border.color: AppTheme.focus

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: root.down
                   ? AppTheme.pressed
                   : (root.hovered ? AppTheme.hover : "transparent")
        }

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }
}
