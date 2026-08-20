pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import "../Singletons"

AbstractButton {
    id: root

    property color onTrackColor: AppTheme.primary
    property color offTrackColor: AppTheme.surfaceVariant
    property color onThumbColor: AppTheme.darkMode ? AppTheme.onPrimary : "#FFFFFF"
    property color offThumbColor: AppTheme.textSecondary
    property color trackBorderColor: AppTheme.outline

    checkable: true

    implicitWidth: 46
    implicitHeight: 26

    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    Accessible.role: Accessible.Button
    Accessible.name: text
    Accessible.checked: checked

    contentItem: Item {}

    background: Rectangle {
        id: _track

        radius: height / 2
        color: root.checked ? root.onTrackColor : root.offTrackColor
        border.width: root.checked ? 0 : 1
        border.color: root.trackBorderColor

        Behavior on color {
            ColorAnimation { duration: 140 }
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: root.down
                   ? AppTheme.pressed
                   : (root.hovered ? AppTheme.hover : "transparent")
        }

        Rectangle {
            id: _thumb

            y: (parent.height - height) / 2
            x: root.checked
               ? parent.width - width - 3
               : 3
            width: parent.height - 6
            height: parent.height - 6
            radius: height / 2
            color: root.checked ? root.onThumbColor : root.offThumbColor

            Behavior on x {
                NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
            }
            Behavior on color {
                ColorAnimation { duration: 140 }
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: root.visualFocus
            color: "transparent"
            border.width: 2
            border.color: AppTheme.focus
        }
    }
}
