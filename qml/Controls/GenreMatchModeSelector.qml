pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    enum MatchMode {
        Or,
        And
    }

    property int selectedMode: GenreMatchModeSelector.Or
    property string orText: qsTr("Match Any")
    property string andText: qsTr("Match All")

    signal modeActivated(int mode)

    implicitWidth: _segments.implicitWidth
    implicitHeight: _segments.implicitHeight

    SegmentedSelector {
        id: _segments

        anchors.fill: parent
        selectedValue: root.selectedMode
        options: [
            {
                label: root.orText,
                value: GenreMatchModeSelector.Or
            },
            {
                label: root.andText,
                value: GenreMatchModeSelector.And
            }
        ]

        onOptionActivated: function(value) {
            const mode = Number(value)
            root.selectedMode = mode
            root.modeActivated(mode)
        }
    }
}
