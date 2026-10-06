pragma ComponentBehavior: Bound

import QtQuick
import "../Singletons"

Item {
    id: root

    property string actionText
    property var _actionCallback: null

    function show(text, actionText, actionCallback) {
        _label.text = text
        root.actionText = actionText !== undefined ? actionText : ""
        root._actionCallback = actionCallback !== undefined ? actionCallback : null
        _hideTimer.interval = root.hasAction ? 4000 : 1800
        opacity = 1
        _hideTimer.restart()
    }

    function dismiss() {
        _hideTimer.stop()
        opacity = 0
        root._actionCallback = null
    }

    readonly property bool hasAction: actionText.length > 0 && _actionCallback !== null

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
        onTriggered: root.dismiss()
    }

    Rectangle {
        id: _pill

        width: _row.implicitWidth + 2 * AppTheme.spacing16
        height: Math.max(_label.implicitHeight + 2 * AppTheme.spacing10,
                         root.hasAction ? AppTheme.touchTargetMinimum : 0)
        radius: AppTheme.radiusPill
        color: AppTheme.surfaceRaised
        border.width: 1
        border.color: AppTheme.outline

        Row {
            id: _row

            anchors.centerIn: parent
            spacing: AppTheme.spacing16

            Text {
                id: _label

                anchors.verticalCenter: parent.verticalCenter
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fs13
                font.weight: Font.DemiBold
            }

            Text {
                id: _action
                objectName: "toastActionButton"

                anchors.verticalCenter: parent.verticalCenter
                visible: root.hasAction
                text: root.actionText
                color: AppTheme.primary
                font.pixelSize: AppTheme.fs13
                font.weight: Font.Black
                font.capitalization: Font.AllUppercase
                font.letterSpacing: 1
                opacity: _actionArea.pressed ? 0.6 : 1.0

                Accessible.role: Accessible.Button
                Accessible.name: root.actionText

                MouseArea {
                    id: _actionArea

                    anchors.fill: parent
                    anchors.margins: -AppTheme.spacing12
                    enabled: root.hasAction
                    cursorShape: Qt.PointingHandCursor

                    onClicked: {
                        const callback = root._actionCallback
                        root.dismiss()
                        if (callback)
                            callback()
                    }
                }
            }
        }
    }
}
