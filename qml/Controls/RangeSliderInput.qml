pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import "../Singletons"

Item {
    id: root

    property string labelText
    property string anyText: qsTr("Any")
    property string separatorText: qsTr(" - ")
    property int from: 1900
    property int to: 2026

    readonly property int firstValue: Math.round(_range.first.value)
    readonly property int secondValue: Math.round(_range.second.value)

    readonly property bool lowerUnbounded: firstValue <= from
    readonly property bool upperUnbounded: secondValue >= to
    readonly property int effectiveMinimum: lowerUnbounded ? 0 : firstValue
    readonly property int effectiveMaximum: upperUnbounded ? 0 : secondValue
    readonly property bool unset: lowerUnbounded && upperUnbounded
    readonly property string valueText:
        (lowerUnbounded ? anyText : String(firstValue))
        + separatorText
        + (upperUnbounded ? anyText : String(secondValue))

    signal rangeEdited(int minimumYear, int maximumYear)
    signal rangeCommitted(int minimumYear, int maximumYear)

    function setRangeSilently(minimumYear, maximumYear) {
        _range.first.value = minimumYear > 0 ? minimumYear : root.from
        _range.second.value = maximumYear > 0 ? maximumYear : root.to
    }

    implicitWidth: 240
    implicitHeight: _label.implicitHeight
                    + AppTheme.spacing6
                    + _range.implicitHeight

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

    Basic.RangeSlider {
        id: _range

        objectName: "rangeSliderInputSlider"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _label.bottom
        anchors.topMargin: AppTheme.spacing6
        from: root.from
        to: root.to
        stepSize: 1
        snapMode: Basic.RangeSlider.SnapAlways
        implicitHeight: AppTheme.controlHeightSmall

        first.value: root.from
        second.value: root.to

        first.onMoved: root.rangeEdited(root.effectiveMinimum,
                                        root.effectiveMaximum)
        second.onMoved: root.rangeEdited(root.effectiveMinimum,
                                         root.effectiveMaximum)
        first.onPressedChanged: {
            if (!first.pressed)
                root.rangeCommitted(root.effectiveMinimum,
                                    root.effectiveMaximum)
        }
        second.onPressedChanged: {
            if (!second.pressed)
                root.rangeCommitted(root.effectiveMinimum,
                                    root.effectiveMaximum)
        }

        background: Rectangle {
            x: _range.leftPadding
            y: _range.topPadding + (_range.availableHeight - height) / 2
            width: _range.availableWidth
            height: 3
            radius: 2
            color: AppTheme.outline

            Rectangle {
                x: _range.first.visualPosition * parent.width
                width: _range.second.visualPosition * parent.width - x
                height: parent.height
                radius: parent.radius
                color: AppTheme.primary
            }
        }

        first.handle: Rectangle {
            objectName: "rangeSliderFirstHandle"
            x: _range.leftPadding
               + _range.first.visualPosition * (_range.availableWidth - width)
            y: _range.topPadding + (_range.availableHeight - height) / 2
            width: 18
            height: 18
            radius: width / 2
            color: AppTheme.primary
            border.width: _range.first.visualFocus ? 2 : 0
            border.color: AppTheme.focus
        }

        second.handle: Rectangle {
            objectName: "rangeSliderSecondHandle"
            x: _range.leftPadding
               + _range.second.visualPosition * (_range.availableWidth - width)
            y: _range.topPadding + (_range.availableHeight - height) / 2
            width: 18
            height: 18
            radius: width / 2
            color: AppTheme.primary
            border.width: _range.second.visualFocus ? 2 : 0
            border.color: AppTheme.focus
        }
    }
}
