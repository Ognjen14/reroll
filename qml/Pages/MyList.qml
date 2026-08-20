pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons" as S
import "../Controls"
import "../Drawers"
import com.topicdev.reroll 1.0

Item {
    id: root

    readonly property var filterOptions: [
        { label: qsTr("All"), value: 0 },
        { label: qsTr("Watchlist"), value: 1 },
        { label: qsTr("Watched"), value: 2 },
        { label: qsTr("Movies"), value: 3 },
        { label: qsTr("TV Shows"), value: 4 },
        { label: qsTr("Hidden"), value: 5 }
    ]

    Rectangle {
        anchors.fill: parent
        color: S.AppTheme.background
    }

    TitleDetailsDrawer {
        id: _titleDetailsDrawer
        objectName: "myListTitleDetailsDrawer"
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
                text: qsTr("MY LIST")
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs28
                font.weight: Font.Black
                elide: Text.ElideRight
            }

            TmdbAttribution {
                objectName: "myListTmdbAttribution"
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: -S.AppTheme.spacing8
            text: {
                MyListController.revision
                return qsTr("%1 titles").arg(MyListController.totalCount())
            }
            color: S.AppTheme.textSecondary
            font.pixelSize: S.AppTheme.fs14
        }

        ListView {
            id: _filterRow
            objectName: "myListFilterRow"

            Layout.fillWidth: true
            Layout.preferredHeight: S.AppTheme.controlHeightMedium
            orientation: ListView.Horizontal
            spacing: S.AppTheme.spacing8
            clip: true
            model: root.filterOptions

            delegate: AppButton {
                id: _filterChip

                required property var modelData

                objectName: "myListFilterChip" + modelData.value
                text: modelData.label
                accessibleName: modelData.label
                contentRadius: S.AppTheme.radiusPill
                backgroundColor: MyListController.filteredModel.mode === modelData.value
                                 ? S.AppTheme.primary
                                 : S.AppTheme.surfaceVariant
                foregroundColor: MyListController.filteredModel.mode === modelData.value
                                 ? (S.AppTheme.darkMode ? S.AppTheme.onPrimary : "#FFFFFF")
                                 : S.AppTheme.textPrimary
                borderColor: MyListController.filteredModel.mode === modelData.value
                             ? "transparent"
                             : S.AppTheme.outline

                onClicked: MyListController.filteredModel.mode = modelData.value
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: _grid.count === 0

            Text {
                anchors.centerIn: parent
                width: Math.min(parent.width - 2 * S.AppTheme.spacing18, 320)
                horizontalAlignment: Text.AlignHCenter
                text: {
                    MyListController.revision
                    if (MyListController.filteredModel.mode === 5)
                        return qsTr("No hidden titles.")
                    return MyListController.totalCount() === 0
                        ? qsTr("Nothing here yet. Add titles from Reroll.")
                        : qsTr("No titles in this filter yet.")
                }
                color: S.AppTheme.textSecondary
                wrapMode: Text.Wrap
            }
        }

        GridView {
            id: _grid
            objectName: "myListGrid"

            readonly property int columns: Math.max(2, Math.floor(width / 130))

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: count > 0
            clip: true
            cellWidth: width / columns
            cellHeight: cellWidth * S.AppTheme.posterAspectRatio + 34
            model: MyListController.filteredModel

            delegate: Item {
                id: _card

                required property var tmdbId
                required property int mediaType
                required property string title
                required property int releaseYear
                required property var genreIds
                required property string posterPath
                required property double rating
                required property var voteCount
                required property bool watchlist
                required property bool watched

                readonly property bool isHiddenTab: MyListController.filteredModel.mode === 5
                readonly property bool canMoveToWatched:
                    !isHiddenTab && watchlist && !watched
                readonly property bool canMoveToWatchlist:
                    !isHiddenTab && watched && !watchlist

                function removeFromWatchlist() {
                    MyListController.setWatchlist(
                        tmdbId, mediaType, title, releaseYear, genreIds,
                        posterPath, rating, voteCount, false)
                    _toast.show(qsTr("Removed from watchlist"))
                }

                function removeFromWatched() {
                    MyListController.setWatched(
                        tmdbId, mediaType, title, releaseYear, genreIds,
                        posterPath, rating, voteCount, false)
                    _toast.show(qsTr("Removed from watched"))
                }

                function moveToWatched() {
                    MyListController.setWatchlist(
                        tmdbId, mediaType, title, releaseYear, genreIds,
                        posterPath, rating, voteCount, false)
                    MyListController.setWatched(
                        tmdbId, mediaType, title, releaseYear, genreIds,
                        posterPath, rating, voteCount, true)
                    _toast.show(qsTr("Moved to watched"))
                }

                function moveToWatchlist() {
                    MyListController.setWatched(
                        tmdbId, mediaType, title, releaseYear, genreIds,
                        posterPath, rating, voteCount, false)
                    MyListController.setWatchlist(
                        tmdbId, mediaType, title, releaseYear, genreIds,
                        posterPath, rating, voteCount, true)
                    _toast.show(qsTr("Moved to watchlist"))
                }

                width: _grid.cellWidth
                height: _grid.cellHeight

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: S.AppTheme.spacing6
                    spacing: S.AppTheme.spacing6

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: width * S.AppTheme.posterAspectRatio

                        MouseArea {
                            objectName: "myListOpenDetailsArea"

                            anchors.fill: parent
                            enabled: !_card.isHiddenTab
                            cursorShape: Qt.PointingHandCursor
                            onClicked: _titleDetailsDrawer.openFor(
                                _card.tmdbId, _card.mediaType, _card.title,
                                _card.releaseYear, _card.posterPath, _card.rating)
                        }

                        PosterImage {
                            anchors.fill: parent
                            opacity: _card.isHiddenTab ? 0.55 : 1.0
                            source: PosterUrlResolver.resolveUrl(_card.posterPath, 342)
                        }

                        Rectangle {
                            objectName: "myListWatchlistBadge"

                            visible: !_card.isHiddenTab && _card.watchlist
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.margins: S.AppTheme.spacing6
                            width: 24
                            height: 24
                            radius: 12
                            color: S.AppTheme.info
                            opacity: _watchlistBadgeArea.pressed ? 0.6 : 1.0

                            ThemedIcon {
                                anchors.centerIn: parent
                                width: 13
                                height: 13
                                source: "qrc:/assets/reroll_page/whichlisted.png"
                                tintColor: "white"
                            }

                            MouseArea {
                                id: _watchlistBadgeArea

                                anchors.fill: parent
                                anchors.margins: -8
                                onClicked: _card.removeFromWatchlist()
                            }
                        }

                        Rectangle {
                            objectName: "myListWatchedBadge"

                            visible: !_card.isHiddenTab && _card.watched
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: S.AppTheme.spacing6
                            width: 24
                            height: 24
                            radius: 12
                            color: S.AppTheme.success
                            opacity: _watchedBadgeArea.pressed ? 0.6 : 1.0

                            ThemedIcon {
                                anchors.centerIn: parent
                                width: 13
                                height: 13
                                source: "qrc:/assets/reroll_page/marked_watched.png"
                                tintColor: "white"
                            }

                            MouseArea {
                                id: _watchedBadgeArea

                                anchors.fill: parent
                                anchors.margins: -8
                                onClicked: _card.removeFromWatched()
                            }
                        }

                        Rectangle {
                            objectName: "myListMoveBar"

                            visible: _card.canMoveToWatched || _card.canMoveToWatchlist
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 22
                            color: Qt.rgba(0, 0, 0, 0.65)
                            opacity: _moveBarArea.pressed ? 0.75 : 1.0

                            Text {
                                anchors.centerIn: parent
                                text: _card.canMoveToWatched
                                      ? qsTr("Mark as watched")
                                      : qsTr("Add to watchlist")
                                color: "white"
                                font.pixelSize: S.AppTheme.fs10
                                font.weight: Font.DemiBold
                            }

                            MouseArea {
                                id: _moveBarArea

                                anchors.fill: parent
                                onClicked: _card.canMoveToWatched
                                           ? _card.moveToWatched()
                                           : _card.moveToWatchlist()
                            }
                        }

                        Rectangle {
                            objectName: "myListRestoreHint"

                            visible: _card.isHiddenTab
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 22
                            color: Qt.rgba(0, 0, 0, 0.65)

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Tap to restore")
                                color: "white"
                                font.pixelSize: S.AppTheme.fs10
                                font.weight: Font.DemiBold
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: _card.isHiddenTab

                            onClicked: {
                                MyListController.setHidden(
                                    _card.tmdbId,
                                    _card.mediaType,
                                    _card.title,
                                    _card.releaseYear,
                                    _card.genreIds,
                                    _card.posterPath,
                                    _card.rating,
                                    _card.voteCount,
                                    false)
                                _toast.show(qsTr("Removed from hidden"))
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: _card.title
                        color: _card.isHiddenTab ? S.AppTheme.textSecondary : S.AppTheme.textPrimary
                        font.pixelSize: S.AppTheme.fs13
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                        maximumLineCount: 1
                    }
                }
            }
        }
    }

    Toast {
        id: _toast
        objectName: "myListToast"

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: S.AppTheme.spacing24
    }
}
