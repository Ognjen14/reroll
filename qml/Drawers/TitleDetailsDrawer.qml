pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import "../Controls"
import "../Singletons"
import com.topicdev.reroll 1.0

Basic.Drawer {
    id: root

    property var tmdbId: 0
    property int mediaType: 0
    property string title
    property int releaseYear: 0
    property string posterPath
    property double rating: 0.0
    property var genreIds: []
    property var voteCount: 0

    readonly property bool inWatchlist: {
        MyListController.revision
        return root.tmdbId > 0 && MyListController.isInWatchlist(root.tmdbId, root.mediaType)
    }
    readonly property bool isWatched: {
        MyListController.revision
        return root.tmdbId > 0 && MyListController.isMarkedWatched(root.tmdbId, root.mediaType)
    }
    readonly property color activeForeground: AppTheme.darkMode ? AppTheme.onPrimary : "#FFFFFF"

    readonly property bool isTv: mediaType === 1
    readonly property string mediaTypeText: isTv ? qsTr("TV") : qsTr("Movie")
    readonly property string yearText: releaseYear > 0 ? releaseYear.toString() : ""
    readonly property string ratingText: rating > 0 ? rating.toFixed(1) : ""
    readonly property bool isCurrentTitle: DiscoverController.titleDetailsTmdbId === tmdbId

    edge: Qt.BottomEdge
    implicitWidth: 460
    implicitHeight: 640
    width: parent ? parent.width : implicitWidth
    height: parent
            ? Math.min(parent.height * 0.9, implicitHeight)
            : implicitHeight
    modal: true
    dim: true
    interactive: false
    closePolicy: Basic.Popup.CloseOnEscape
                 | Basic.Popup.CloseOnPressOutside

    function openFor(newTmdbId, newMediaType, newTitle, newReleaseYear,
                      newPosterPath, newRating, newGenreIds, newVoteCount) {
        root.tmdbId = newTmdbId
        root.mediaType = newMediaType
        root.title = newTitle
        root.releaseYear = newReleaseYear
        root.posterPath = newPosterPath
        root.rating = newRating
        root.genreIds = newGenreIds !== undefined && newGenreIds !== null ? newGenreIds : []
        root.voteCount = newVoteCount !== undefined && newVoteCount !== null ? newVoteCount : 0
        DiscoverController.loadTitleDetails(newTmdbId, newMediaType)
        root.open()
    }

    onOpened: PopupRegistry.register(root)
    onClosed: PopupRegistry.unregister(root)

    background: Rectangle {
        color: AppTheme.surface
        radius: AppTheme.radiusSheet
        border.width: 1
        border.color: AppTheme.outline

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.radius
            color: parent.color
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 44

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: AppTheme.spacing8
                width: 36
                height: 4
                radius: 2
                color: AppTheme.outlineStrong
            }

            AppButton {
                objectName: "titleDetailsCloseButton"

                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: AppTheme.spacing12
                anchors.topMargin: AppTheme.spacing4
                shape: AppButton.CircleShape
                shapeSize: 32
                imageSource: "qrc:/assets/reroll_page/x.png"
                imageSize: 14
                backgroundColor: AppTheme.surfaceVariant
                foregroundColor: AppTheme.textSecondary
                accessibleName: qsTr("Close")

                onClicked: root.close()
            }
        }

        Basic.ScrollView {
            id: _scrollView

            objectName: "titleDetailsScrollView"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            Column {
                property real horizontalInset: AppTheme.spacing18

                width: _scrollView.availableWidth
                spacing: AppTheme.spacing16

                RowLayout {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    spacing: AppTheme.spacing14

                    Item {
                        Layout.preferredWidth: 96
                        Layout.preferredHeight: 96 * AppTheme.posterAspectRatio
                        Layout.alignment: Qt.AlignTop

                        PosterImage {
                            anchors.fill: parent
                            source: PosterUrlResolver.resolveUrl(root.posterPath, 342)
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        spacing: AppTheme.spacing8

                        Text {
                            Layout.fillWidth: true
                            text: root.title
                            color: AppTheme.textPrimary
                            font.pixelSize: AppTheme.fs18
                            font.weight: Font.Black
                            wrapMode: Text.Wrap
                            maximumLineCount: 3
                            elide: Text.ElideRight
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: AppTheme.spacing6

                            GenreTag {
                                text: root.mediaTypeText
                                backgroundColor: AppTheme.surfaceVariant
                                foregroundColor: AppTheme.textSecondary
                                borderColor: "transparent"
                            }

                            GenreTag {
                                visible: root.yearText.length > 0
                                text: root.yearText
                                backgroundColor: AppTheme.surfaceVariant
                                foregroundColor: AppTheme.textSecondary
                                borderColor: "transparent"
                            }

                            GenreTag {
                                visible: root.ratingText.length > 0
                                text: root.ratingText
                                iconSource: "qrc:/assets/start_rating.png"
                                backgroundColor: AppTheme.surfaceVariant
                                foregroundColor: AppTheme.textSecondary
                                borderColor: "transparent"
                            }
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: AppTheme.spacing6
                            visible: root.isCurrentTitle
                                     && DiscoverController.titleDetailsGenreNames.length > 0

                            Repeater {
                                model: root.isCurrentTitle
                                       ? DiscoverController.titleDetailsGenreNames
                                       : []

                                delegate: GenreTag {
                                    required property string modelData

                                    text: modelData
                                    backgroundColor: Qt.rgba(
                                        AppTheme.primary.r, AppTheme.primary.g,
                                        AppTheme.primary.b, 0.12)
                                    foregroundColor: AppTheme.primary
                                    borderColor: "transparent"
                                }
                            }
                        }
                    }
                }

                Row {
                    objectName: "titleDetailsListActions"

                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    spacing: AppTheme.spacing8

                    AppButton {
                        objectName: "titleDetailsWatchlistButton"

                        width: (parent.width - parent.spacing) / 2
                        height: AppTheme.controlHeightMedium
                        text: root.inWatchlist ? qsTr("On watchlist") : qsTr("Watchlist")
                        accessibleName: root.inWatchlist
                                        ? qsTr("Remove from watchlist")
                                        : qsTr("Add to watchlist")
                        imageSource: root.inWatchlist
                                     ? "qrc:/assets/reroll_page/whichlisted.png"
                                     : "qrc:/assets/reroll_page/whichlist.png"
                        imageSize: 18
                        contentRadius: AppTheme.radiusPill
                        backgroundColor: root.inWatchlist ? AppTheme.primary : AppTheme.surfaceVariant
                        foregroundColor: root.inWatchlist ? root.activeForeground : AppTheme.textPrimary
                        borderColor: root.inWatchlist ? "transparent" : AppTheme.outline

                        Accessible.checked: root.inWatchlist

                        onClicked: MyListController.setWatchlist(
                            root.tmdbId, root.mediaType, root.title, root.releaseYear,
                            root.genreIds, root.posterPath, root.rating, root.voteCount,
                            !root.inWatchlist)
                    }

                    AppButton {
                        objectName: "titleDetailsWatchedButton"

                        width: (parent.width - parent.spacing) / 2
                        height: AppTheme.controlHeightMedium
                        text: root.isWatched ? qsTr("Watched") : qsTr("Mark watched")
                        accessibleName: root.isWatched
                                        ? qsTr("Unmark as watched")
                                        : qsTr("Mark as watched")
                        imageSource: root.isWatched
                                     ? "qrc:/assets/reroll_page/marked_watched.png"
                                     : "qrc:/assets/reroll_page/mark_watched.png"
                        imageSize: 18
                        contentRadius: AppTheme.radiusPill
                        backgroundColor: root.isWatched ? AppTheme.primary : AppTheme.surfaceVariant
                        foregroundColor: root.isWatched ? root.activeForeground : AppTheme.textPrimary
                        borderColor: root.isWatched ? "transparent" : AppTheme.outline

                        Accessible.checked: root.isWatched

                        onClicked: MyListController.setWatched(
                            root.tmdbId, root.mediaType, root.title, root.releaseYear,
                            root.genreIds, root.posterPath, root.rating, root.voteCount,
                            !root.isWatched)
                    }
                }

                Column {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    spacing: AppTheme.spacing8

                    Text {
                        text: qsTr("OVERVIEW")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fs12
                        font.weight: Font.Black
                        font.letterSpacing: 1
                    }

                    Text {
                        width: parent.width
                        text: {
                            if (!root.isCurrentTitle || DiscoverController.titleDetailsLoading)
                                return qsTr("Loading...")
                            const overview = DiscoverController.titleDetailsOverview
                            return overview.length > 0
                                   ? overview
                                   : qsTr("No overview is available for this title.")
                        }
                        color: AppTheme.textPrimary
                        font.pixelSize: AppTheme.fs14
                        lineHeight: 1.4
                        wrapMode: Text.Wrap
                    }
                }

                TmdbAttribution {
                    x: parent.horizontalInset
                    objectName: "titleDetailsTmdbAttribution"
                    compact: true
                }

                Column {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    spacing: AppTheme.spacing8
                    visible: root.isCurrentTitle
                             && DiscoverController.titleDetailsStreamingProviders.length > 0

                    Text {
                        text: qsTr("WHERE TO WATCH")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fs12
                        font.weight: Font.Black
                        font.letterSpacing: 1
                    }

                    Flow {
                        width: parent.width
                        spacing: AppTheme.spacing8

                        Repeater {
                            model: root.isCurrentTitle
                                   ? DiscoverController.titleDetailsStreamingProviders
                                   : []

                            delegate: Row {
                                required property var modelData

                                spacing: AppTheme.spacing6

                                Rectangle {
                                    width: 28
                                    height: 28
                                    radius: AppTheme.radiusSmall
                                    color: AppTheme.surfaceVariant

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 3
                                        source: modelData.logoUrl
                                        fillMode: Image.PreserveAspectFit
                                        asynchronous: true
                                        smooth: true
                                    }
                                }

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.name
                                    color: AppTheme.textPrimary
                                    font.pixelSize: AppTheme.fs12
                                    font.weight: Font.Medium
                                }
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        text: qsTr("Streaming availability shown for the US - it may differ where you are.")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fs10
                        wrapMode: Text.Wrap
                    }
                }

                AppButton {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    height: AppTheme.controlHeightLarge
                    text: qsTr("Play Trailer")
                    accessibleName: qsTr("Play trailer")
                    contentRadius: AppTheme.radiusPill
                    backgroundColor: AppTheme.surfaceVariant
                    foregroundColor: AppTheme.textPrimary
                    borderColor: "transparent"
                    enabled: !DiscoverController.titleDetailsTrailerLoading

                    onClicked: DiscoverController.playTitleDetailsTrailer()
                }

                Item {
                    width: 1
                    height: 20
                }
            }
        }
    }
}
