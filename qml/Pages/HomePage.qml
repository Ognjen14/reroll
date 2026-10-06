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
                                        && HomeController.state !== HomeController.Empty
    readonly property bool showingStatePanel: !suggestionReady || !hasSuggestion
    readonly property int wideLayoutThreshold: S.AppTheme.breakpointTablet
    readonly property bool wideLayout: width >= wideLayoutThreshold
    readonly property int defaultMediaType: 0
    property real dragOffsetX: 0
    readonly property real dragFade: width > 0
                                     ? Math.min(Math.abs(dragOffsetX) / width, 0.5)
                                     : 0
    readonly property int activeFilterCount: {
        let count = 0
        if (HomeController.appliedMediaType !== root.defaultMediaType)
            count++
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

    readonly property var filterSummary: {
        const items = []
        switch (HomeController.appliedMediaType) {
        case 1:
            items.push(qsTr("TV"))
            break
        case 2:
            items.push(qsTr("Movies & TV"))
            break
        default:
            items.push(qsTr("Movies"))
            break
        }

        const minimumYear = HomeController.appliedMinimumYear
        const maximumYear = HomeController.appliedMaximumYear
        if (minimumYear > 0 && maximumYear > 0)
            items.push(minimumYear === maximumYear
                       ? minimumYear.toString()
                       : qsTr("%1-%2").arg(minimumYear).arg(maximumYear))
        else if (minimumYear > 0)
            items.push(qsTr("From %1").arg(minimumYear))
        else if (maximumYear > 0)
            items.push(qsTr("Up to %1").arg(maximumYear))

        if (HomeController.appliedMinimumRating > 0)
            items.push(qsTr("%1+ rating").arg(HomeController.appliedMinimumRating.toFixed(1)))

        const genreNames = HomeController.appliedGenreNames
        const visibleGenreCount = genreNames.length > 3 ? 2 : genreNames.length
        for (let i = 0; i < visibleGenreCount; ++i)
            items.push(genreNames[i])
        if (genreNames.length > visibleGenreCount)
            items.push(qsTr("+%1 genres").arg(genreNames.length - visibleGenreCount))
        else if (genreNames.length === 0 && HomeController.appliedGenreCount > 0)
            items.push(qsTr("%n genre(s)", "", HomeController.appliedGenreCount))

        if (HomeController.appliedExcludeWatched)
            items.push(qsTr("Unwatched"))

        return items
    }

    function openDetails() {
        if (!HomeController.hasSuggestion)
            return
        _detailsDrawer.openFor(HomeController.currentTmdbId,
                               HomeController.isTv ? 1 : 0,
                               HomeController.title,
                               HomeController.releaseYear,
                               HomeController.currentPosterPath,
                               HomeController.rating,
                               HomeController.currentGenreIds,
                               HomeController.voteCount)
    }

    function requestReroll() {
        if (!HomeController.canReroll)
            return
        _actionBar.spinReroll()
        HomeController.reroll()
    }

    function requestPrevious() {
        if (!HomeController.previous())
            _toast.show(qsTr("No previous suggestion"))
    }

    function hideCurrent() {
        const tmdbId = HomeController.currentTmdbId
        const mediaType = HomeController.isTv ? 1 : 0
        const title = HomeController.title
        const releaseYear = HomeController.releaseYear
        const genreIds = HomeController.currentGenreIds
        const posterPath = HomeController.currentPosterPath
        const rating = HomeController.rating
        const voteCount = HomeController.voteCount

        MyListController.setHidden(tmdbId, mediaType, title, releaseYear, genreIds,
                                   posterPath, rating, voteCount, true)
        HomeController.reroll()
        _toast.show(qsTr("Won't show this title again"), qsTr("Undo"), function() {
            MyListController.setHidden(tmdbId, mediaType, title, releaseYear, genreIds,
                                       posterPath, rating, voteCount, false)
            HomeController.previous()
        })
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

    Connections {
        target: HomeController

        function onMoreTitlesFailed() {
            _toast.show(qsTr("Couldn't load more titles. Tap Reroll to try again."))
        }
    }

    Rectangle {
        anchors.fill: parent
        color: S.AppTheme.background
    }

    FilterDrawer {
        id: _filterDrawer
    }

    TitleDetailsDrawer {
        id: _detailsDrawer
        objectName: "homeTitleDetailsDrawer"
    }

    NumberAnimation {
        id: _settleAnimation

        target: root
        property: "dragOffsetX"
        to: 0
        duration: 180
        easing.type: Easing.OutCubic
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
                opacity: 1 - root.dragFade

                transform: Translate {
                    x: root.dragOffsetX * 0.5
                }
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

            MouseArea {
                id: _swipeArea
                objectName: "homeSwipeArea"

                property real startX: 0
                property real startY: 0
                readonly property real swipeThreshold: Math.min(width * 0.22, 120)
                readonly property real tapTolerance: 12

                anchors.fill: parent
                enabled: HomeController.hasSuggestion && root.suggestionReady
                preventStealing: true
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                onPressed: function(mouse) {
                    _settleAnimation.stop()
                    startX = mouse.x
                    startY = mouse.y
                    root.dragOffsetX = 0
                }

                onPositionChanged: function(mouse) {
                    const dx = mouse.x - startX
                    const dy = mouse.y - startY
                    root.dragOffsetX = Math.abs(dx) > Math.abs(dy) ? dx : 0
                }

                onReleased: function(mouse) {
                    const dx = mouse.x - startX
                    const dy = mouse.y - startY
                    _settleAnimation.start()

                    if (Math.abs(dx) < tapTolerance && Math.abs(dy) < tapTolerance) {
                        root.openDetails()
                        return
                    }

                    if (Math.abs(dx) > Math.abs(dy)) {
                        if (dx <= -swipeThreshold)
                            root.requestReroll()
                        else if (dx >= swipeThreshold)
                            root.requestPrevious()
                    } else if (dy <= -swipeThreshold) {
                        root.openDetails()
                    }
                }

                onCanceled: _settleAnimation.start()
            }

            RowLayout {
                id: _heroHeader
                objectName: "homeHeroHeader"

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.leftMargin: S.AppTheme.spacing18
                anchors.rightMargin: S.AppTheme.spacing18
                anchors.topMargin: S.AppTheme.spacing20
                spacing: S.AppTheme.spacing12

                Text {
                    Layout.alignment: Qt.AlignTop
                    text: qsTr("REROLL")
                    color: HomeController.hasSuggestion ? "white" : S.AppTheme.textPrimary
                    font.pixelSize: S.AppTheme.fs22
                    font.weight: Font.Black
                    font.letterSpacing: 1
                }

                Item {
                    Layout.fillWidth: true
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: S.AppTheme.spacing6

                    RowLayout {
                        Layout.alignment: Qt.AlignRight
                        spacing: S.AppTheme.spacing6

                        AppButton {
                            objectName: "homePreviousButton"

                            visible: HomeController.canGoBack && HomeController.hasSuggestion
                            enabled: root.suggestionReady
                            text: qsTr("Previous")
                            accessibleName: qsTr("Show previous suggestion")
                            contentRadius: S.AppTheme.radiusPill
                            backgroundColor: HomeController.hasSuggestion
                                             ? Qt.rgba(1, 1, 1, 0.14)
                                             : S.AppTheme.surfaceVariant
                            foregroundColor: HomeController.hasSuggestion
                                             ? "white"
                                             : S.AppTheme.textPrimary
                            borderColor: "transparent"

                            onClicked: root.requestPrevious()
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
                }
            }

            Flow {
                id: _filterSummary
                objectName: "homeFilterSummary"

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: _heroHeader.bottom
                anchors.leftMargin: S.AppTheme.spacing18
                anchors.rightMargin: S.AppTheme.spacing18
                anchors.topMargin: S.AppTheme.spacing10
                spacing: S.AppTheme.spacing6
                visible: root.filterSummary.length > 0
                opacity: _filterSummaryArea.pressed ? 0.6 : 1.0

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Active filters: %1").arg(root.filterSummary.join(", "))

                Repeater {
                    model: root.filterSummary

                    delegate: GenreTag {
                        required property string modelData

                        text: modelData
                        backgroundColor: HomeController.hasSuggestion
                                         ? Qt.rgba(0, 0, 0, 0.35)
                                         : S.AppTheme.surfaceVariant
                        foregroundColor: HomeController.hasSuggestion
                                         ? "white"
                                         : S.AppTheme.textSecondary
                        borderColor: HomeController.hasSuggestion
                                     ? Qt.rgba(1, 1, 1, 0.2)
                                     : S.AppTheme.outline
                    }
                }
            }

            MouseArea {
                id: _filterSummaryArea
                objectName: "homeFilterSummaryArea"

                x: _filterSummary.x
                y: _filterSummary.y
                width: _filterSummary.childrenRect.width
                height: _filterSummary.childrenRect.height
                visible: _filterSummary.visible
                cursorShape: Qt.PointingHandCursor

                onClicked: _filterDrawer.open()
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
                opacity: 1 - root.dragFade
                wideLayout: root.wideLayout
                recycled: HomeController.recycled
                tmdbId: HomeController.currentTmdbId
                title: HomeController.title
                year: HomeController.releaseYear
                mediaType: HomeController.isTv ? SuggestionCard.Tv : SuggestionCard.Movie
                rating: HomeController.rating
                overview: HomeController.overview
                genreNames: HomeController.currentGenreNames
                streamingProviders: HomeController.currentStreamingProviders

                transform: Translate {
                    x: root.dragOffsetX * 0.5
                }
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
                              || mode === StatePanel.Empty
                retryText: mode === StatePanel.Empty ? qsTr("Edit filters") : qsTr("Try again")
                secondaryVisible: mode === StatePanel.Empty
                                  && !HomeController.appliedFiltersAreDefault
                secondaryText: qsTr("Reset filters")

                onRetryRequested: {
                    if (mode === StatePanel.Empty)
                        _filterDrawer.open()
                    else
                        HomeController.retry()
                }

                onSecondaryRequested: {
                    if (HomeController.resetAndApply())
                        _toast.show(qsTr("Filters reset"))
                }
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
            onHideRequested: root.hideCurrent()
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
