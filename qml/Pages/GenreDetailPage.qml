import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../Singletons" as S
import "../Controls"
import "../Drawers"
import com.topicdev.reroll 1.0

Item {
    id: root

    property string genreName
    property var titleModel

    signal loadMoreRequested()

    Rectangle {
        anchors.fill: parent
        color: S.AppTheme.background
    }

    TitleDetailsDrawer {
        id: _titleDetailsDrawer
        objectName: "genreDetailTitleDetailsDrawer"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: S.AppTheme.spacing18
        spacing: S.AppTheme.spacing12

        RowLayout {
            Layout.fillWidth: true
            spacing: S.AppTheme.spacing12

            AppButton {
                objectName: "genreDetailBackButton"

                text: qsTr("Back")
                accessibleName: qsTr("Back to Discover")
                backgroundColor: S.AppTheme.surfaceVariant
                foregroundColor: S.AppTheme.textPrimary
                borderColor: "transparent"

                onClicked: root.StackView.view.pop()
            }

            Text {
                Layout.fillWidth: true
                text: root.genreName
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs22
                font.weight: Font.Black
                elide: Text.ElideRight
            }

            TmdbAttribution {
                objectName: "genreDetailTmdbAttribution"
            }
        }

        GridView {
            id: _grid
            objectName: "genreDetailGrid"

            readonly property int columns: Math.max(2, Math.floor(width / 130))

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: width / columns
            cellHeight: cellWidth * S.AppTheme.posterAspectRatio + 48
            model: root.titleModel

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
                    root.loadMoreRequested()
                }
            }

            onContentHeightChanged: Qt.callLater(maybeLoadMore)
            onContentYChanged: Qt.callLater(maybeLoadMore)

            delegate: DiscoverPosterCardDelegate {
                width: _grid.cellWidth

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
                width: _grid.width
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
    }

    Toast {
        id: _toast
        objectName: "genreDetailToast"

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: S.AppTheme.spacing24
    }
}
