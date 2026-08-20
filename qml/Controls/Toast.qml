pragma ComponentBehavior: Bound

import QtQuick
import "../Singletons"

Item {
    id: root

    function show(text) {
        _label.text = text
        opacity = 1
        _hideTimer.restart()
    }

    width: _pill.width
    height: _pill.height
    opacity: 0
    visible: opacity > 0.01

    Behavior on opacity {
        NumberAnimation { duration: 180 }
    }

    Timer {
        id: _hideTimer

        interval: 1800
        onTriggered: root.opacity = 0
    }

    Rectangle {
        id: _pill

        width: _label.implicitWidth + 2 * AppTheme.spacing16
        height: _label.implicitHeight + 2 * AppTheme.spacing10
        radius: AppTheme.radiusPill
        color: AppTheme.surfaceRaised
        border.width: 1
        border.color: AppTheme.outline

        Text {
            id: _label

            anchors.centerIn: parent
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fs13
            font.weight: Font.DemiBold
        }
    }
}
