pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    enum MediaType {
        Movie,
        Tv,
        Both
    }

    property int selectedMediaType: MediaTypeSelector.Movie
    property string movieText: qsTr("Movies")
    property string tvText: qsTr("TV shows")
    property string bothText: qsTr("Both")

    signal mediaTypeActivated(int mediaType)

    implicitWidth: _segments.implicitWidth
    implicitHeight: _segments.implicitHeight

    SegmentedSelector {
        id: _segments

        anchors.fill: parent
        selectedValue: root.selectedMediaType
        options: [
            {
                label: root.movieText,
                value: MediaTypeSelector.Movie
            },
            {
                label: root.tvText,
                value: MediaTypeSelector.Tv
            },
            {
                label: root.bothText,
                value: MediaTypeSelector.Both
            }
        ]

        onOptionActivated: function(value) {
            const mediaType = Number(value)
            root.selectedMediaType = mediaType
            root.mediaTypeActivated(mediaType)
        }
    }
}
