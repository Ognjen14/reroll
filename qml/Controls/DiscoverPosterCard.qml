pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons"
import com.topicdev.reroll 1.0

Item {
    id: root

    property var tmdbId
    property int mediaType: 0
    property string title
    property int releaseYear: 0
    property var genreIds: []
    property string posterPath
    property double rating: 0.0
    property var voteCount: 0

    signal watchlistToggled(bool newValue)
    signal watchedToggled(bool newValue)
    signal detailsRequested()

    readonly property bool watchlist: {
        MyListController.revision
        return MyListController.isInWatchlist(root.tmdbId, root.mediaType)
    }
    readonly property bool watched: {
        MyListController.revision
        return MyListController.isMarkedWatched(root.tmdbId, root.mediaType)
    }
    readonly property string mediaTypeText: root.mediaType === 1 ? qsTr("TV") : qsTr("Movie")
    readonly property string yearText: root.releaseYear > 0 ? root.releaseYear.toString() : ""
    readonly property string metaText: [mediaTypeText, yearText]
                                       .filter(value => value.length > 0)
                                       .join("  ·  ")
    readonly property string ratingText: root.rating > 0 ? root.rating.toFixed(1) : ""

    implicitWidth: 140
    implicitHeight: _content.implicitHeight
    height: implicitHeight

    MouseArea {
        objectName: "discoverPosterCardOpenArea"

        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.detailsRequested()
    }

    ColumnLayout {
        id: _content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: AppTheme.spacing6

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: width * AppTheme.posterAspectRatio

            PosterImage {
                anchors.fill: parent
                source: PosterUrlResolver.resolveUrl(root.posterPath, 342)
            }

            Rectangle {
                objectName: "discoverRatingBadge"

                visible: root.ratingText.length > 0
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: AppTheme.spacing6
                width: _ratingRow.implicitWidth + 2 * AppTheme.spacing6
                height: _ratingRow.implicitHeight + 2 * AppTheme.spacing4
                radius: AppTheme.radiusSmall
                color: Qt.rgba(0, 0, 0, 0.72)

                Row {
                    id: _ratingRow

                    anchors.centerIn: parent
                    spacing: AppTheme.spacing4

                    Image {
                        width: 11
                        height: 11
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/assets/start_rating.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    Text {
                        id: _ratingText

                        anchors.verticalCenter: parent.verticalCenter
                        text: root.ratingText
                        color: "white"
                        font.pixelSize: AppTheme.fs11
                        font.weight: Font.Bold
                    }
                }
            }

            AppButton {
                objectName: "discoverWatchlistBadge"

                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: AppTheme.spacing6
                shape: AppButton.CircleShape
                shapeSize: 26
                imageSource: root.watchlist
                             ? "qrc:/assets/reroll_page/whichlisted.png"
                             : "qrc:/assets/reroll_page/whichlist.png"
                imageSize: 13
                accessibleName: root.watchlist
                                ? qsTr("Remove from watchlist")
                                : qsTr("Add to watchlist")
                backgroundColor: root.watchlist ? AppTheme.info : Qt.rgba(0, 0, 0, 0.55)
                foregroundColor: "white"
                borderColor: "transparent"

                onClicked: {
                    const newValue = !root.watchlist
                    MyListController.setWatchlist(
                        root.tmdbId,
                        root.mediaType,
                        root.title,
                        root.releaseYear,
                        root.genreIds,
                        root.posterPath,
                        root.rating,
                        root.voteCount,
                        newValue)
                    root.watchlistToggled(newValue)
                }
            }

            AppButton {
                objectName: "discoverWatchedBadge"

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: AppTheme.spacing6
                shape: AppButton.CircleShape
                shapeSize: 26
                imageSource: root.watched
                             ? "qrc:/assets/reroll_page/marked_watched.png"
                             : "qrc:/assets/reroll_page/mark_watched.png"
                imageSize: 13
                accessibleName: root.watched
                                ? qsTr("Remove from watched")
                                : qsTr("Mark as watched")
                backgroundColor: root.watched ? AppTheme.success : Qt.rgba(0, 0, 0, 0.55)
                foregroundColor: "white"
                borderColor: "transparent"

                onClicked: {
                    const newValue = !root.watched
                    MyListController.setWatched(
                        root.tmdbId,
                        root.mediaType,
                        root.title,
                        root.releaseYear,
                        root.genreIds,
                        root.posterPath,
                        root.rating,
                        root.voteCount,
                        newValue)
                    root.watchedToggled(newValue)
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: root.title
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fs13
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            maximumLineCount: 1
        }

        Text {
            Layout.fillWidth: true
            visible: root.metaText.length > 0
            text: root.metaText
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fs11
            elide: Text.ElideRight
        }
    }
}
