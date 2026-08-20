pragma ComponentBehavior: Bound

import QtQuick
import "../Singletons"

Item {
    id: root

    property var options: []
    property var selectedValue
    property real cornerRadius: AppTheme.radiusXLarge
    property color backgroundColor: AppTheme.surfaceVariant
    property color borderColor: "transparent"

    readonly property int optionCount: options ? options.length : 0

    signal optionActivated(var value)

    function isSelected(value) {
        return selectedValue === value
    }

    implicitWidth: Math.max(200, optionCount * 100)
    implicitHeight: AppTheme.controlHeightMedium + 6

    Rectangle {
        anchors.fill: parent
        radius: root.cornerRadius
        color: root.backgroundColor
        border.width: root.borderColor.a > 0 ? 1 : 0
        border.color: root.borderColor
        clip: true

        Row {
            id: _optionRow

            anchors.fill: parent
            anchors.margins: 3

            Repeater {
                model: root.options

                delegate: SegmentedOption {
                    required property int index
                    required property var modelData

                    objectName: "segmentedOption" + index
                    width: root.optionCount > 0
                           ? _optionRow.width / root.optionCount
                           : 0
                    height: _optionRow.height
                    text: modelData.label
                    selected: root.isSelected(modelData.value)
                    enabled: root.enabled && modelData.enabled !== false

                    onClicked: {
                        if (!selected)
                            root.optionActivated(modelData.value)
                    }
                }
            }
        }
    }
}
