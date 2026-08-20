pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import "../Singletons"

Item {
    id: root

    property string labelText
    property string unsetText: qsTr("No minimum")
    property real from: 0
    property real to: 10
    property real stepSize: 0.5
    property real unsetValue: 0
    property int decimals: 1

    readonly property real value: _slider.value

    readonly property bool unset: value <= unsetValue
    readonly property string valueText:
        unset
        ? unsetText
        : formatValue(value)

    signal valueEdited(real value)
    signal valueCommitted(real value)

    function formatValue(candidate) {
        return decimals > 0
               ? Number(candidate).toFixed(decimals)
               : Number(Math.round(candidate)).toLocaleString(Qt.locale(), "f", 0)
    }

    function setValueSilently(candidate) {
        _slider.value = candidate
    }

    implicitWidth: 240
    implicitHeight: _label.implicitHeight
                    + AppTheme.spacing6
                    + _slider.implicitHeight

    Text {
        id: _label

        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width - _valueLabel.width - AppTheme.spacing8
        text: root.labelText
        color: AppTheme.textSecondary
        font.pixelSize: AppTheme.fs12
        elide: Text.ElideRight
    }

    Text {
        id: _valueLabel

        anchors.right: parent.right
        anchors.baseline: _label.baseline
        text: root.valueText
        color: root.unset ? AppTheme.textSecondary : AppTheme.primary
        font.pixelSize: AppTheme.fs12
        font.weight: Font.DemiBold
    }

    Basic.Slider {
        id: _slider

        objectName: "sliderInputSlider"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _label.bottom
        anchors.topMargin: AppTheme.spacing6
        from: root.from
        to: root.to
        stepSize: root.stepSize
        snapMode: Basic.Slider.SnapAlways
        implicitHeight: AppTheme.controlHeightSmall

        onMoved: root.valueEdited(value)
        onPressedChanged: {
            if (!pressed)
                root.valueCommitted(value)
        }

        background: Rectangle {
            x: _slider.leftPadding
            y: _slider.topPadding + (_slider.availableHeight - height) / 2
            width: _slider.availableWidth
            height: 3
            radius: 2
            color: AppTheme.outline

            Rectangle {
                width: _slider.visualPosition * parent.width
                height: parent.height
                radius: parent.radius
                color: AppTheme.primary
            }
        }

        handle: Rectangle {
            x: _slider.leftPadding
               + _slider.visualPosition * (_slider.availableWidth - width)
            y: _slider.topPadding + (_slider.availableHeight - height) / 2
            width: 18
            height: 18
            radius: width / 2
            color: AppTheme.primary
            border.width: _slider.visualFocus ? 2 : 0
            border.color: AppTheme.focus
        }
    }
}
