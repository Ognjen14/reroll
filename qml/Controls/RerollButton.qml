import QtQuick
import "../Singletons"

AppButton {
    id: root

    property bool busy: false
    property int spinTurns: 3
    property int spinDurationPerTurn: 520

    readonly property bool spinning: _spin.running || _continuous.running

    function spin() {
        if (_continuous.running)
            return
        _spin.restart()
    }

    imageSize: 22
    contentRadius: AppTheme.radiusLarge
    imagePlaceholderText: qsTr("RR")

    onBusyChanged: {
        if (busy) {
            _spin.stop()
            _continuous.restart()
        } else {
            _continuous.stop()
            root.iconItem.rotation = 0
        }
    }

    RotationAnimation {
        id: _spin

        target: root.iconItem
        property: "rotation"
        from: 0
        to: 360 * root.spinTurns
        duration: root.spinDurationPerTurn * root.spinTurns
        easing.type: Easing.OutCubic

        onStopped: root.iconItem.rotation = 0
    }

    RotationAnimation {
        id: _continuous

        target: root.iconItem
        property: "rotation"
        from: 0
        to: 360
        duration: root.spinDurationPerTurn
        loops: Animation.Infinite
        easing.type: Easing.Linear
    }
}
