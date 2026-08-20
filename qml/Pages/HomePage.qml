import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import "../Singletons" as S
import "../Controls"
import "../Drawers"
import com.topicdev.reroll 1.0

Item {
    id: root

    readonly property bool suggestionReady: HomeController.state === HomeController.Ready
    readonly property bool hasSuggestion: HomeController.hasSuggestion
    readonly property bool cardVisible: hasSuggestion
    readonly property bool showingStatePanel: !suggestionReady || !hasSuggestion
    readonly property int wideLayoutThreshold: S.AppTheme.breakpointTablet
    readonly property bool wideLayout: width >= wideLayoutThreshold
    readonly property int activeFilterCount: {
        let count = 0
        if (HomeController.appliedMinimumYear > 0 || HomeController.appliedMaximumYear > 0)
            count++
        if (HomeController.appliedMinimumRating > 0)
            count++
        if (HomeController.appliedGenreCount > 0)
            count++
        if (HomeController.appliedExcludeWatched)
            count++
        return count
    }

    function statePanelMode() {
        switch (HomeController.state) {
        case HomeController.Empty:
            return StatePanel.Empty
        case HomeController.NetworkError:
            return StatePanel.NetworkError
        case HomeController.RateLimited:
            return StatePanel.RateLimited
        default:
            return StatePanel.Loading
        }
    }

    Rectangle {
        anchors.fill: parent
        color: S.AppTheme.background
    }

    FilterDrawer {
        id: _filterDrawer
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            id: _hero
            objectName: "homeHero"

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Rectangle {
                anchors.fill: parent
                color: HomeController.hasSuggestion ? "#0B0C0E" : S.AppTheme.background

                Behavior on color {
                    ColorAnimation { duration: 200 }
                }
            }

            Image {
                id: _heroBackdropFill

                anchors.fill: parent
                source: HomeController.posterUrl
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                visible: false
            }

            MultiEffect {
                anchors.fill: parent
                source: _heroBackdropFill
                visible: HomeController.hasSuggestion
                autoPaddingEnabled: false
                blurEnabled: true
                blur: 1.0
                blurMax: 64
                brightness: -0.3
                saturation: -0.15
            }

            Image {
                id: _heroBackdropSharp

                anchors.fill: parent
                source: HomeController.posterUrl
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                cache: true
                visible: HomeController.hasSuggestion
            }

            Rectangle {
                visible: HomeController.hasSuggestion
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: parent.height * 0.28

                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.6) }
                    GradientStop { position: 1.0; color: "transparent" }
                }
            }

            Rectangle {
                visible: HomeController.hasSuggestion
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: parent.height * 0.55

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 0.5; color: Qt.rgba(0, 0, 0, 0.55) }
                    GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.92) }
                }
            }

            RowLayout {
                objectName: "homeHeroHeader"

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.leftMargin: S.AppTheme.spacing18
                anchors.rightMargin: S.AppTheme.spacing18
                anchors.topMargin: S.AppTheme.spacing20
                spacing: S.AppTheme.spacing12

                Text {
                    text: qsTr("REROLL")
                    color: HomeController.hasSuggestion ? "white" : S.AppTheme.textPrimary
                    font.pixelSize: S.AppTheme.fs22
                    font.weight: Font.Black
                    font.letterSpacing: 1
                }

                Item {
                    Layout.fillWidth: true
                }

                AppButton {
                    objectName: "filtersHeaderButton"

                    text: qsTr("Filters")
                    accessibleName: qsTr("Open filters")
                    contentRadius: S.AppTheme.radiusPill
                    backgroundColor: HomeController.hasSuggestion
                                     ? Qt.rgba(1, 1, 1, 0.14)
                                     : S.AppTheme.surfaceVariant
                    foregroundColor: HomeController.hasSuggestion
                                     ? "white"
                                     : S.AppTheme.textPrimary
                    borderColor: "transparent"

                    onClicked: _filterDrawer.open()

                    Rectangle {
                        objectName: "filtersHeaderBadge"

                        visible: root.activeFilterCount > 0
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.rightMargin: -2
                        anchors.topMargin: -2
                        width: 18
                        height: 18
                        radius: 9
                        color: S.AppTheme.error

                        Text {
                            anchors.centerIn: parent
                            text: root.activeFilterCount
                            color: "white"
                            font.pixelSize: S.AppTheme.fs10
                            font.weight: Font.Bold
                        }
                    }
                }
            }

            SuggestionCard {
                id: _suggestionCard
                objectName: "homeSuggestionCard"

                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: root.wideLayout
                       ? Math.min(parent.width - 2 * S.AppTheme.spacing32, 960)
                       : parent.width
                visible: root.cardVisible
                wideLayout: root.wideLayout
                recycled: HomeController.recycled
                title: HomeController.title
                year: HomeController.releaseYear
                mediaType: HomeController.isTv ? SuggestionCard.Tv : SuggestionCard.Movie
                rating: HomeController.rating
                overview: HomeController.overview
                genreNames: HomeController.currentGenreNames
                streamingProviders: HomeController.currentStreamingProviders
            }

            StatePanel {
                id: _statePanel
                objectName: "homeStatePanel"

                anchors.centerIn: parent
                width: Math.min(parent.width - 2 * S.AppTheme.spacing18, 640)
                visible: root.showingStatePanel
                mode: root.statePanelMode()
                messageText: HomeController.errorText.length > 0
                             ? HomeController.errorText
                             : defaultMessageText
                retryVisible: mode === StatePanel.NetworkError
                              || mode === StatePanel.RateLimited

                onRetryRequested: HomeController.retry()
            }
        }

        SuggestionActionBar {
            id: _actionBar
            objectName: "homeActionBar"

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: S.AppTheme.spacing12
            Layout.bottomMargin: S.AppTheme.spacing16
            visible: root.cardVisible
            actionsEnabled: root.suggestionReady
            trailerLoading: HomeController.trailerLoading
            rerollEnabled: HomeController.canReroll
            rerollBusy: HomeController.replenishing
            watchlist: {
                MyListController.revision
                return HomeController.hasSuggestion
                    ? MyListController.isInWatchlist(
                          HomeController.currentTmdbId, HomeController.isTv ? 1 : 0)
                    : false
            }
            watched: {
                MyListController.revision
                return HomeController.hasSuggestion
                    ? MyListController.isMarkedWatched(
                          HomeController.currentTmdbId, HomeController.isTv ? 1 : 0)
                    : false
            }

            onWatchlistToggleRequested: {
                const newValue = !_actionBar.watchlist
                MyListController.setWatchlist(
                    HomeController.currentTmdbId,
                    HomeController.isTv ? 1 : 0,
                    HomeController.title,
                    HomeController.releaseYear,
                    HomeController.currentGenreIds,
                    HomeController.currentPosterPath,
                    HomeController.rating,
                    HomeController.voteCount,
                    newValue)
                _toast.show(newValue ? qsTr("Added to watchlist") : qsTr("Removed from watchlist"))
            }

            onWatchedToggleRequested: {
                const newValue = !_actionBar.watched
                MyListController.setWatched(
                    HomeController.currentTmdbId,
                    HomeController.isTv ? 1 : 0,
                    HomeController.title,
                    HomeController.releaseYear,
                    HomeController.currentGenreIds,
                    HomeController.currentPosterPath,
                    HomeController.rating,
                    HomeController.voteCount,
                    newValue)
                _toast.show(newValue ? qsTr("Marked as watched") : qsTr("Removed from watched"))
            }

            onTrailerRequested: HomeController.playTrailer()
            onRerollRequested: HomeController.reroll()

            onHideRequested: {
                MyListController.setHidden(
                    HomeController.currentTmdbId,
                    HomeController.isTv ? 1 : 0,
                    HomeController.title,
                    HomeController.releaseYear,
                    HomeController.currentGenreIds,
                    HomeController.currentPosterPath,
                    HomeController.rating,
                    HomeController.voteCount,
                    true)
                _toast.show(qsTr("Won't show this title again"))
                HomeController.reroll()
            }
        }
    }

    Toast {
        id: _toast
        objectName: "homeToast"

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: S.AppTheme.spacing16 + _actionBar.height + S.AppTheme.spacing12
    }
}
