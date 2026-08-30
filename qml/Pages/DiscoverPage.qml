pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import "../Singletons" as S
import "../Controls"
import "../Drawers"
import com.topicdev.reroll 1.0

Item {
    id: root

    readonly property bool searchActive: DiscoverController.searchQuery.trim().length > 0
    property bool genresExpanded: false
    property bool tvGenresActive: false

    readonly property bool hasDiscoverContent: _trendingMoviesRow.count > 0
                                                || _trendingTvRow.count > 0
                                                || _popularMoviesRow.count > 0
                                                || _popularTvRow.count > 0
                                                || _genreRepeater.count > 0

    readonly property int genrePreviewCount: 6

    readonly property var searchScopeOptions: [
        { label: qsTr("All"), value: 0 },
        { label: qsTr("Movies"), value: 1 },
        { label: qsTr("TV"), value: 2 }
    ]

    readonly property var genreGradients: [
        ["#8B2942", "#4A1625"],
        ["#1F6F78", "#12333A"],
        ["#A15A1E", "#4A2A0E"],
        ["#5B3E9E", "#2E1F52"],
        ["#2F7D4F", "#153822"],
        ["#A13030", "#4A1414"],
        ["#2A5C99", "#132A48"],
        ["#8E3E7A", "#3F1A37"]
    ]

    Component {
        id: _titleGridComponent

        GenreDetailPage {}
    }

    function openTitleGrid(name, titleModel, onLoadMore) {
        const pushedItem = root.StackView.view.push(_titleGridComponent, {
            genreName: name,
            titleModel: titleModel
        })
        if (onLoadMore) {
            pushedItem.loadMoreRequested.connect(onLoadMore)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: S.AppTheme.background
    }

    TitleDetailsDrawer {
        id: _titleDetailsDrawer
        objectName: "discoverTitleDetailsDrawer"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: S.AppTheme.spacing18
        spacing: S.AppTheme.spacing12

        RowLayout {
            Layout.fillWidth: true
            spacing: S.AppTheme.spacing8

            Text {
                Layout.fillWidth: true
                text: qsTr("DISCOVER")
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs28
                font.weight: Font.Black
                elide: Text.ElideRight
            }

            TmdbAttribution {
                objectName: "discoverTmdbAttribution"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: S.AppTheme.controlHeightMedium
            radius: S.AppTheme.radiusPill
            color: S.AppTheme.surfaceVariant
            border.width: _searchField.activeFocus ? 1 : 0
            border.color: S.AppTheme.primary

            Basic.TextField {
                id: _searchField
                objectName: "discoverSearchField"

                anchors.fill: parent
                anchors.leftMargin: S.AppTheme.spacing16
                anchors.rightMargin: S.AppTheme.spacing16
                verticalAlignment: TextInput.AlignVCenter
                placeholderText: qsTr("Search movies & TV")
                color: S.AppTheme.textPrimary
                placeholderTextColor: S.AppTheme.textSecondary
                font.pixelSize: S.AppTheme.fs14
                text: DiscoverController.searchQuery
                background: Item {}

                onTextEdited: DiscoverController.searchQuery = text
            }
        }

        ListView {
            id: _scopeRow
            objectName: "discoverSearchScopeRow"

            visible: root.searchActive
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? S.AppTheme.controlHeightMedium : 0
            orientation: ListView.Horizontal
            spacing: S.AppTheme.spacing8
            clip: true
            model: root.searchScopeOptions

            delegate: AppButton {
                id: _scopeChip

                required property var modelData

                objectName: "discoverSearchScopeChip" + modelData.value
                text: modelData.label
                accessibleName: modelData.label
                contentRadius: S.AppTheme.radiusPill
                backgroundColor: DiscoverController.searchScope === modelData.value
                                 ? S.AppTheme.primary
                                 : S.AppTheme.surfaceVariant
                foregroundColor: DiscoverController.searchScope === modelData.value
                                 ? (S.AppTheme.darkMode ? S.AppTheme.onPrimary : "#FFFFFF")
                                 : S.AppTheme.textPrimary
                borderColor: DiscoverController.searchScope === modelData.value
                             ? "transparent"
                             : S.AppTheme.outline

                onClicked: DiscoverController.searchScope = modelData.value
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 48
            horizontalAlignment: Text.AlignHCenter
            visible: root.searchActive
                     && !DiscoverController.searching
                     && _searchGrid.count === 0
            text: qsTr("No results for “%1”").arg(DiscoverController.searchQuery)
            color: S.AppTheme.textSecondary
            wrapMode: Text.Wrap
        }

        GridView {
            id: _searchGrid
            objectName: "discoverSearchGrid"

            readonly property int columns: Math.max(2, Math.floor(width / 130))

            visible: root.searchActive && count > 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: width / columns
            cellHeight: cellWidth * S.AppTheme.posterAspectRatio + 48
            model: DiscoverController.searchResults

            ScrollBar.vertical: ScrollBar {
                id: _scrollBar

                policy: ScrollBar.AsNeeded

                background: Rectangle {
                    implicitWidth: 6
                    radius: width / 2
                    color: S.AppTheme.outline
                    opacity: 0.4
                }

                contentItem: Rectangle {
                    implicitWidth: 6
                    radius: width / 2
                    color: S.AppTheme.primary
                    opacity: _scrollBar.pressed ? 1.0 : 0.7

                    Behavior on opacity { NumberAnimation { duration: 120 } }
                }
            }

            function maybeLoadMore() {
                if (contentHeight <= height
                    || contentY + height >= contentHeight - cellHeight) {
                    DiscoverController.loadMoreSearchResults()
                }
            }

            onContentHeightChanged: Qt.callLater(maybeLoadMore)
            onContentYChanged: Qt.callLater(maybeLoadMore)

            delegate: DiscoverPosterCardDelegate {
                width: _searchGrid.cellWidth

                onWatchlistToggled: function(newValue) {
                    _toast.show(newValue
                                ? qsTr("Added to watchlist")
                                : qsTr("Removed from watchlist"))
                }
                onWatchedToggled: function(newValue) {
                    _toast.show(newValue
                                ? qsTr("Marked as watched")
                                : qsTr("Removed from watched"))
                }
                onDetailsRequested: _titleDetailsDrawer.openFor(
                    tmdbId, mediaType, title, releaseYear, posterPath, rating)
            }

            footer: Item {
                width: _searchGrid.width
                height: DiscoverController.loadingMore ? 60 : 0

                BusyIndicator {
                    anchors.centerIn: parent
                    width: 28
                    height: 28
                    visible: DiscoverController.loadingMore
                    running: visible
                }
            }
        }

        ScrollView {
            visible: !root.searchActive
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Column {
                width: root.width - 2 * S.AppTheme.spacing18
                spacing: S.AppTheme.spacing20

                Column {
                    width: parent.width
                    spacing: S.AppTheme.spacing10

                    RowLayout {
                        width: parent.width
                        spacing: S.AppTheme.spacing8

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("BROWSE BY GENRE")
                            color: S.AppTheme.textPrimary
                            font.pixelSize: S.AppTheme.fs14
                            font.weight: Font.Black
                            font.letterSpacing: 1
                        }

                        AppButton {
                            objectName: "discoverGenreScopeMovies"

                            text: qsTr("Movies")
                            accessibleName: text
                            contentRadius: S.AppTheme.radiusPill
                            backgroundColor: !root.tvGenresActive
                                             ? S.AppTheme.primary
                                             : S.AppTheme.surfaceVariant
                            foregroundColor: !root.tvGenresActive
                                             ? (S.AppTheme.darkMode ? S.AppTheme.onPrimary : "#FFFFFF")
                                             : S.AppTheme.textPrimary
                            borderColor: !root.tvGenresActive ? "transparent" : S.AppTheme.outline

                            onClicked: root.tvGenresActive = false
                        }

                        AppButton {
                            objectName: "discoverGenreScopeTv"

                            text: qsTr("TV")
                            accessibleName: text
                            contentRadius: S.AppTheme.radiusPill
                            backgroundColor: root.tvGenresActive
                                             ? S.AppTheme.primary
                                             : S.AppTheme.surfaceVariant
                            foregroundColor: root.tvGenresActive
                                             ? (S.AppTheme.darkMode ? S.AppTheme.onPrimary : "#FFFFFF")
                                             : S.AppTheme.textPrimary
                            borderColor: root.tvGenresActive ? "transparent" : S.AppTheme.outline

                            onClicked: root.tvGenresActive = true
                        }
                    }

                    GridLayout {
                        width: parent.width
                        columns: 2
                        columnSpacing: S.AppTheme.spacing10
                        rowSpacing: S.AppTheme.spacing10

                        Repeater {
                            id: _genreRepeater

                            model: root.tvGenresActive
                                   ? DiscoverController.tvGenreSections
                                   : DiscoverController.genreSections

                            delegate: Rectangle {
                                id: _genreCard

                                required property int index
                                required property var genreId
                                required property string name
                                required property var titleModel
                                required property var totalResults

                                readonly property var gradientPair:
                                    root.genreGradients[_genreCard.index % root.genreGradients.length]
                                readonly property bool previewHidden:
                                    !root.genresExpanded && _genreCard.index >= root.genrePreviewCount

                                visible: !previewHidden
                                Layout.fillWidth: true
                                Layout.preferredHeight: previewHidden ? 0 : 90
                                radius: S.AppTheme.radiusLarge
                                clip: true

                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: _genreCard.gradientPair[0] }
                                    GradientStop { position: 1.0; color: _genreCard.gradientPair[1] }
                                }

                                Column {
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.margins: S.AppTheme.spacing12
                                    spacing: 2

                                    Text {
                                        width: parent.width
                                        text: _genreCard.name.toUpperCase()
                                        color: "white"
                                        font.pixelSize: S.AppTheme.fs14
                                        font.weight: Font.Black
                                        wrapMode: Text.Wrap
                                    }

                                    Text {
                                        visible: _genreCard.totalResults > 0
                                        text: qsTr("%1 titles").arg(_genreCard.totalResults)
                                        color: Qt.rgba(1, 1, 1, 0.85)
                                        font.pixelSize: S.AppTheme.fs11
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    enabled: !_genreCard.previewHidden
                                    onClicked: {
                                        const isTv = root.tvGenresActive
                                        root.openTitleGrid(
                                            _genreCard.name,
                                            _genreCard.titleModel,
                                            function() {
                                                if (isTv) {
                                                    DiscoverController.loadMoreForTvGenre(
                                                        _genreCard.genreId)
                                                } else {
                                                    DiscoverController.loadMoreForGenre(
                                                        _genreCard.genreId)
                                                }
                                            })
                                    }
                                }
                            }
                        }
                    }

                    AppButton {
                        objectName: "discoverGenresToggleButton"

                        visible: _genreRepeater.count > root.genrePreviewCount
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: root.genresExpanded ? qsTr("Show less") : qsTr("Show all genres")
                        accessibleName: text
                        backgroundColor: S.AppTheme.surfaceVariant
                        foregroundColor: S.AppTheme.textPrimary
                        borderColor: "transparent"

                        onClicked: root.genresExpanded = !root.genresExpanded
                    }
                }

                Column {
                    width: parent.width
                    spacing: S.AppTheme.spacing10
                    visible: _trendingMoviesRow.count > 0

                    RowLayout {
                        width: parent.width

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("TRENDING NOW")
                            color: S.AppTheme.textPrimary
                            font.pixelSize: S.AppTheme.fs14
                            font.weight: Font.Black
                            font.letterSpacing: 1
                        }

                        Text {
                            objectName: "discoverSeeAllTrending"
                            text: qsTr("See all")
                            color: S.AppTheme.info

                            MouseArea {
                                anchors.fill: parent
                                anchors.margins: -S.AppTheme.spacing8
                                onClicked: root.openTitleGrid(
                                               qsTr("Trending Now"),
                                               DiscoverController.trendingMovies,
                                               function() {
                                                   DiscoverController.loadMoreTrendingMovies()
                                               })
                            }
                        }
                    }

                    ListView {
                        id: _trendingMoviesRow
                        objectName: "discoverTrendingMoviesRow"

                        width: parent.width
                        height: 210
                        orientation: ListView.Horizontal
                        spacing: S.AppTheme.spacing10
                        clip: true
                        model: DiscoverController.trendingMovies

                        delegate: DiscoverPosterCardDelegate {
                            width: 140

                            onWatchlistToggled: function(newValue) {
                                _toast.show(newValue
                                            ? qsTr("Added to watchlist")
                                            : qsTr("Removed from watchlist"))
                            }
                            onWatchedToggled: function(newValue) {
                                _toast.show(newValue
                                            ? qsTr("Marked as watched")
                                            : qsTr("Removed from watched"))
                            }
                            onDetailsRequested: _titleDetailsDrawer.openFor(
                                tmdbId, mediaType, title, releaseYear, posterPath, rating)
                        }
                    }
                }

                Column {
                    width: parent.width
                    spacing: S.AppTheme.spacing10
                    visible: _trendingTvRow.count > 0

                    Text {
                        text: qsTr("TRENDING TV")
                        color: S.AppTheme.textPrimary
                        font.pixelSize: S.AppTheme.fs14
                        font.weight: Font.Black
                        font.letterSpacing: 1
                    }

                    ListView {
                        id: _trendingTvRow
                        objectName: "discoverTrendingTvRow"

                        width: parent.width
                        height: 210
                        orientation: ListView.Horizontal
                        spacing: S.AppTheme.spacing10
                        clip: true
                        model: DiscoverController.trendingTv

                        delegate: DiscoverPosterCardDelegate {
                            width: 140

                            onWatchlistToggled: function(newValue) {
                                _toast.show(newValue
                                            ? qsTr("Added to watchlist")
                                            : qsTr("Removed from watchlist"))
                            }
                            onWatchedToggled: function(newValue) {
                                _toast.show(newValue
                                            ? qsTr("Marked as watched")
                                            : qsTr("Removed from watched"))
                            }
                            onDetailsRequested: _titleDetailsDrawer.openFor(
                                tmdbId, mediaType, title, releaseYear, posterPath, rating)
                        }
                    }
                }

                Column {
                    width: parent.width
                    spacing: S.AppTheme.spacing10
                    visible: _popularMoviesRow.count > 0

                    Text {
                        text: qsTr("POPULAR MOVIES")
                        color: S.AppTheme.textPrimary
                        font.pixelSize: S.AppTheme.fs14
                        font.weight: Font.Black
                        font.letterSpacing: 1
                    }

                    ListView {
                        id: _popularMoviesRow
                        objectName: "discoverPopularMoviesRow"

                        width: parent.width
                        height: 210
                        orientation: ListView.Horizontal
                        spacing: S.AppTheme.spacing10
                        clip: true
                        model: DiscoverController.popularMovies

                        delegate: DiscoverPosterCardDelegate {
                            width: 140

                            onWatchlistToggled: function(newValue) {
                                _toast.show(newValue
                                            ? qsTr("Added to watchlist")
                                            : qsTr("Removed from watchlist"))
                            }
                            onWatchedToggled: function(newValue) {
                                _toast.show(newValue
                                            ? qsTr("Marked as watched")
                                            : qsTr("Removed from watched"))
                            }
                            onDetailsRequested: _titleDetailsDrawer.openFor(
                                tmdbId, mediaType, title, releaseYear, posterPath, rating)
                        }
                    }
                }

                Column {
                    width: parent.width
                    spacing: S.AppTheme.spacing10
                    visible: _popularTvRow.count > 0

                    Text {
                        text: qsTr("POPULAR TV")
                        color: S.AppTheme.textPrimary
                        font.pixelSize: S.AppTheme.fs14
                        font.weight: Font.Black
                        font.letterSpacing: 1
                    }

                    ListView {
                        id: _popularTvRow
                        objectName: "discoverPopularTvRow"

                        width: parent.width
                        height: 210
                        orientation: ListView.Horizontal
                        spacing: S.AppTheme.spacing10
                        clip: true
                        model: DiscoverController.popularTv

                        delegate: DiscoverPosterCardDelegate {
                            width: 140

                            onWatchlistToggled: function(newValue) {
                                _toast.show(newValue
                                            ? qsTr("Added to watchlist")
                                            : qsTr("Removed from watchlist"))
                            }
                            onWatchedToggled: function(newValue) {
                                _toast.show(newValue
                                            ? qsTr("Marked as watched")
                                            : qsTr("Removed from watched"))
                            }
                            onDetailsRequested: _titleDetailsDrawer.openFor(
                                tmdbId, mediaType, title, releaseYear, posterPath, rating)
                        }
                    }
                }

                Item {
                    width: 1
                    height: S.AppTheme.spacing18
                }
            }
        }
    }

    StatePanel {
        id: _loadingPanel
        objectName: "discoverLoadingPanel"

        visible: DiscoverController.loading
                 && !root.hasDiscoverContent
                 && !DiscoverController.loadFailed
                 && !root.searchActive
        anchors.centerIn: parent
        width: Math.min(parent.width - 2 * S.AppTheme.spacing18, 420)
        mode: StatePanel.Loading
        titleText: qsTr("Loading Discover")
    }

    StatePanel {
        id: _errorPanel
        objectName: "discoverErrorPanel"

        visible: DiscoverController.loadFailed && !root.searchActive
        anchors.centerIn: parent
        width: Math.min(parent.width - 2 * S.AppTheme.spacing18, 420)
        mode: StatePanel.NetworkError
        titleText: qsTr("Unable to load Discover")
        messageText: qsTr("Check your connection and try again.")
        retryVisible: true

        onRetryRequested: DiscoverController.retryInitialLoad()
    }

    Toast {
        id: _toast
        objectName: "discoverToast"

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: S.AppTheme.spacing24
    }
}
