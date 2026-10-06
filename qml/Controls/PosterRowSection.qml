pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons"

Column {
    id: root

    property string title
    property var model
    property string listObjectName
    property bool seeAllVisible: true
    property real cardWidth: 140
    property real rowHeight: 210
    readonly property alias count: _row.count

    signal seeAllRequested()
    signal watchlistToggled(bool newValue)
    signal watchedToggled(bool newValue)
    signal detailsRequested(var tmdbId, int mediaType, string title,
                            int releaseYear, string posterPath, double rating,
                            var genreIds, var voteCount)

    spacing: AppTheme.spacing10
    visible: _row.count > 0

    RowLayout {
        width: parent.width

        Text {
            Layout.fillWidth: true
            text: root.title
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fs14
            font.weight: Font.Black
            font.letterSpacing: 1
            font.capitalization: Font.AllUppercase
            elide: Text.ElideRight
        }

        Text {
            objectName: root.listObjectName + "SeeAll"

            visible: root.seeAllVisible
            text: qsTr("See all")
            color: AppTheme.info
            opacity: _seeAllArea.pressed ? 0.6 : 1.0

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("See all %1").arg(root.title)

            MouseArea {
                id: _seeAllArea

                anchors.fill: parent
                anchors.margins: -AppTheme.spacing8
                cursorShape: Qt.PointingHandCursor
                onClicked: root.seeAllRequested()
            }
        }
    }

    ListView {
        id: _row
        objectName: root.listObjectName

        width: parent.width
        height: root.rowHeight
        orientation: ListView.Horizontal
        spacing: AppTheme.spacing10
        clip: true
        model: root.model

        delegate: DiscoverPosterCardDelegate {
            id: _delegate

            width: root.cardWidth

            onWatchlistToggled: function(newValue) { root.watchlistToggled(newValue) }
            onWatchedToggled: function(newValue) { root.watchedToggled(newValue) }
            onDetailsRequested: root.detailsRequested(
                _delegate.tmdbId, _delegate.mediaType, _delegate.title,
                _delegate.releaseYear, _delegate.posterPath, _delegate.rating,
                _delegate.genreIds, _delegate.voteCount)
        }
    }
}
