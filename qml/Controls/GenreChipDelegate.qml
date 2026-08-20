pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property int genreId
    required property string name
    required property bool selected

    signal selectionRequested(int genreId, bool selected)

    implicitWidth: _chip.implicitWidth
    implicitHeight: _chip.implicitHeight

    GenreChip {
        id: _chip

        objectName: "genreChip" + root.genreId
        anchors.fill: parent
        genreId: root.genreId
        text: root.name
        selected: root.selected

        onSelectionRequested: function(genreId, selected) {
            root.selectionRequested(genreId, selected)
        }
    }
}
