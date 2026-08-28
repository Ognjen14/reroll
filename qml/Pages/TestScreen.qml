pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Singletons" as S
import "../Controls"
import "../Drawers"
import com.topicdev.reroll 1.0

Item {
    id: root

    property var testSelectedGenres: [28]

    function toggleTestGenre(genreId) {
        const list = root.testSelectedGenres
        const idx = list.indexOf(genreId)
        if (idx === -1) {
            list.push(genreId)
        } else {
            list.splice(idx, 1)
        }
        root.testSelectedGenres = list.slice()
    }

    readonly property var testGenres: [
        { id: 28, name: "Action" },
        { id: 12, name: "Adventure" },
        { id: 35, name: "Comedy" },
        { id: 10759, name: "Action & Adventure" }
    ]

    Rectangle {
        anchors.fill: parent
        color: S.AppTheme.background
    }

    FilterDrawer {
        id: _filterDrawer
    }

    TitleDetailsDrawer {
        id: _titleDetailsDrawer
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: S.AppTheme.spacing18
        clip: true

        Column {
            width: root.width - 2 * S.AppTheme.spacing18
            spacing: S.AppTheme.spacing24

            Text {
                text: "TEST SCREEN"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs28
                font.weight: Font.Black
            }

            Text {
                width: parent.width
                text: "Every reusable Controls/ component, for visual review. Remove this page before release."
                color: S.AppTheme.textSecondary
                font.pixelSize: S.AppTheme.fs12
                wrapMode: Text.Wrap
            }

            // ============================================================
            Text {
                text: "APPBUTTON"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Flow {
                width: parent.width
                spacing: S.AppTheme.spacing10

                AppButton {
                    text: "Primary"
                    accessibleName: text
                }

                AppButton {
                    text: "Secondary"
                    accessibleName: text
                    backgroundColor: S.AppTheme.surfaceVariant
                    foregroundColor: S.AppTheme.textPrimary
                    borderColor: S.AppTheme.outline
                }

                AppButton {
                    text: "Disabled"
                    accessibleName: text
                    enabled: false
                }

                AppButton {
                    text: "Pill"
                    accessibleName: text
                    contentRadius: S.AppTheme.radiusPill
                }

                AppButton {
                    shape: AppButton.CircleShape
                    shapeSize: 44
                    imageSource: "qrc:/assets/reroll_page/reroll.png"
                    imageSize: 22
                    accessibleName: "Circle shape"
                }

                AppButton {
                    shape: AppButton.RoundedSquareShape
                    shapeSize: 44
                    imageSource: "qrc:/assets/reroll_page/whichlist.png"
                    imageSize: 20
                    accessibleName: "Rounded square shape"
                }

                AppButton {
                    shape: AppButton.SquareShape
                    shapeSize: 44
                    imageSource: "qrc:/assets/reroll_page/mark_watched.png"
                    imageSize: 20
                    accessibleName: "Square shape"
                }
            }

            // ============================================================
            Text {
                text: "REROLLBUTTON (tap to spin)"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            RerollButton {
                shape: AppButton.CircleShape
                shapeSize: 56
                imageSource: "qrc:/assets/reroll_page/reroll.png"
                imageSize: 28
                accessibleName: "Reroll"

                onClicked: spin()
            }

            // ============================================================
            Text {
                text: "SEGMENTED SELECTORS"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Column {
                width: parent.width
                spacing: S.AppTheme.spacing12

                SegmentedSelector {
                    width: 260
                    options: [
                        { label: "One", value: 0 },
                        { label: "Two", value: 1 },
                        { label: "Three", value: 2 }
                    ]
                    selectedValue: 0
                }

                MediaTypeSelector {
                    selectedMediaType: MediaTypeSelector.Both
                }

                GenreMatchModeSelector {
                    selectedMode: GenreMatchModeSelector.Or
                }
            }

            // ============================================================
            Text {
                text: "GENRE CHIPS"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Flow {
                width: parent.width
                spacing: S.AppTheme.spacing8

                Repeater {
                    model: root.testGenres

                    delegate: GenreChipDelegate {
                        required property var modelData

                        genreId: modelData.id
                        name: modelData.name
                        selected: root.testSelectedGenres.indexOf(modelData.id) !== -1

                        onSelectionRequested: function(genreId, selected) {
                            root.toggleTestGenre(genreId)
                        }
                    }
                }
            }

            // ============================================================
            Text {
                text: "GENRE TAGS"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Flow {
                width: parent.width
                spacing: S.AppTheme.spacing8

                GenreTag {
                    text: "8.4"
                    iconSource: "qrc:/assets/start_rating.png"
                }

                GenreTag {
                    text: "Movie  ·  2026"
                }

                GenreTag {
                    text: "Drama"
                }
            }

            // ============================================================
            Text {
                text: "SLIDERS"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Column {
                width: Math.min(parent.width, 320)
                spacing: S.AppTheme.spacing20

                SliderInput {
                    width: parent.width
                    labelText: "Minimum rating"
                }

                RangeSliderInput {
                    width: parent.width
                    labelText: "Year range"
                }
            }

            // ============================================================
            Text {
                text: "TOGGLESWITCH"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Row {
                spacing: S.AppTheme.spacing20

                ToggleSwitch {
                    checked: true
                }

                ToggleSwitch {
                    checked: false
                }
            }

            // ============================================================
            Text {
                text: "STATEPANEL"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Flow {
                width: parent.width
                spacing: S.AppTheme.spacing12

                StatePanel {
                    mode: StatePanel.Loading
                }

                StatePanel {
                    mode: StatePanel.Empty
                }

                StatePanel {
                    mode: StatePanel.NetworkError
                    retryVisible: true
                }

                StatePanel {
                    mode: StatePanel.RateLimited
                    retryVisible: true
                }
            }

            // ============================================================
            Text {
                text: "UNAPPLIEDCHANGESNOTICE"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            UnappliedChangesNotice {
                active: true
            }

            // ============================================================
            Text {
                text: "SETTINGSLINKROW"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Rectangle {
                width: parent.width
                height: _linkRow.implicitHeight
                color: S.AppTheme.surface
                radius: S.AppTheme.radiusMedium

                SettingsLinkRow {
                    id: _linkRow
                    width: parent.width
                    text: "Example link row"
                }
            }

            // ============================================================
            Text {
                text: "THEMEDICON"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Row {
                spacing: S.AppTheme.spacing16

                ThemedIcon {
                    width: 28
                    height: 28
                    source: "qrc:/assets/nav/home.svg"
                    tintColor: S.AppTheme.primary
                }

                ThemedIcon {
                    width: 28
                    height: 28
                    source: ""
                    showPlaceholder: true
                    placeholderText: "N/A"
                }
            }

            // ============================================================
            Text {
                text: "TMDBATTRIBUTION"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Column {
                width: parent.width
                spacing: S.AppTheme.spacing12

                TmdbAttribution {
                    compact: true
                }

                TmdbAttribution {
                    compact: false
                    width: parent.width
                }
            }

            // ============================================================
            Text {
                text: "POSTERIMAGE / POSTERPLACEHOLDER"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Row {
                spacing: S.AppTheme.spacing12

                PosterImage {
                    width: 100
                    height: 150
                }

                PosterPlaceholder {
                    width: 100
                    height: 150
                    text: "NO POSTER"
                }
            }

            // ============================================================
            Text {
                text: "TOAST"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Item {
                width: parent.width
                height: Math.max(_toastShowButton.height, _testToast.height)

                AppButton {
                    id: _toastShowButton
                    text: "Show toast"
                    accessibleName: text
                    backgroundColor: S.AppTheme.surfaceVariant
                    foregroundColor: S.AppTheme.textPrimary
                    borderColor: S.AppTheme.outline

                    onClicked: _testToast.show("This is a test toast")
                }

                Toast {
                    id: _testToast
                    anchors.left: _toastShowButton.right
                    anchors.leftMargin: S.AppTheme.spacing12
                    anchors.verticalCenter: _toastShowButton.verticalCenter
                }
            }

            // ============================================================
            Text {
                text: "SUGGESTIONACTIONBAR"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Column {
                width: parent.width
                spacing: S.AppTheme.spacing12

                SuggestionActionBar {
                    watchlist: false
                    watched: false
                }

                SuggestionActionBar {
                    watchlist: true
                    watched: true
                }
            }

            // ============================================================
            Text {
                text: "SUGGESTIONCARD"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Rectangle {
                width: parent.width
                height: _suggestionCard.implicitHeight + S.AppTheme.spacing18
                radius: S.AppTheme.radiusLarge
                color: "#101418"

                SuggestionCard {
                    id: _suggestionCard
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    title: "Example Title"
                    year: 2026
                    mediaType: SuggestionCard.Movie
                    rating: 7.8
                    overview: "A short sample overview to check wrapping, line count, and the overlay text treatment against a dark backdrop."
                    genreNames: ["Action", "Adventure"]
                }
            }

            // ============================================================
            Text {
                text: "DISCOVERPOSTERCARD"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Row {
                spacing: S.AppTheme.spacing12

                DiscoverPosterCardDelegate {
                    width: 140
                    tmdbId: -1001
                    mediaType: 0
                    title: "Sample Movie Title"
                    releaseYear: 2026
                    genreIds: [28]
                    posterPath: ""
                    rating: 6.5
                    voteCount: 1200
                }

                DiscoverPosterCardDelegate {
                    width: 140
                    tmdbId: -1002
                    mediaType: 1
                    title: "Sample TV Show"
                    releaseYear: 2025
                    genreIds: [10759]
                    posterPath: ""
                    rating: 8.1
                    voteCount: 340
                }
            }

            // ============================================================
            Text {
                text: "DRAWERS (tap to open)"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Flow {
                width: parent.width
                spacing: S.AppTheme.spacing10

                AppButton {
                    text: "Open FilterDrawer"
                    accessibleName: text
                    backgroundColor: S.AppTheme.surfaceVariant
                    foregroundColor: S.AppTheme.textPrimary
                    borderColor: S.AppTheme.outline

                    onClicked: _filterDrawer.open()
                }

                AppButton {
                    text: "Open TitleDetailsDrawer"
                    accessibleName: text
                    backgroundColor: S.AppTheme.surfaceVariant
                    foregroundColor: S.AppTheme.textPrimary
                    borderColor: S.AppTheme.outline

                    onClicked: _titleDetailsDrawer.openFor(
                                   -1001, 0, "Sample Movie Title", 2026, "", 6.5)
                }
            }

            // ============================================================
            Text {
                text: "SETTINGS PICKERS"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Column {
                width: parent.width
                spacing: S.AppTheme.spacing16

                ThemePicker {
                    width: parent.width
                }

                AccentPicker {
                    width: parent.width
                }

                FontSizePicker {
                    width: parent.width
                }
            }

            // ============================================================
            Text {
                text: "NAVIGATIONBAR"
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Black
                font.letterSpacing: 1
            }

            Rectangle {
                width: parent.width
                height: 64
                color: "transparent"
                border.width: 1
                border.color: S.AppTheme.outline

                NavigationBar {
                    anchors.fill: parent
                }
            }

            Item {
                width: 1
                height: S.AppTheme.spacing24
            }
        }
    }
}
