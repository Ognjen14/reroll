pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property var tmdbId
    required property int mediaType
    required property string title
    required property int releaseYear
    required property var genreIds
    required property string posterPath
    required property double rating
    required property var voteCount

    signal watchlistToggled(bool newValue)
    signal watchedToggled(bool newValue)
    signal detailsRequested()

    height: _posterCard.height

    DiscoverPosterCard {
        id: _posterCard

        width: root.width
        tmdbId: root.tmdbId
        mediaType: root.mediaType
        title: root.title
        releaseYear: root.releaseYear
        genreIds: root.genreIds
        posterPath: root.posterPath
        rating: root.rating
        voteCount: root.voteCount

        onWatchlistToggled: function(newValue) { root.watchlistToggled(newValue) }
        onWatchedToggled: function(newValue) { root.watchedToggled(newValue) }
        onDetailsRequested: root.detailsRequested()
    }
}
