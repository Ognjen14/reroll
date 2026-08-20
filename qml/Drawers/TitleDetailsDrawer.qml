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
    closePolicy: Basic.Popup.CloseOnEscape
                 | Basic.Popup.CloseOnPressOutside

    function openFor(newTmdbId, newMediaType, newTitle, newReleaseYear,
                      newPosterPath, newRating) {
        root.tmdbId = newTmdbId
        root.mediaType = newMediaType
        root.title = newTitle
        root.releaseYear = newReleaseYear
        root.posterPath = newPosterPath
        root.rating = newRating
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
            Layout.preferredHeight: AppTheme.spacing20

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: AppTheme.spacing8
                width: 36
                height: 4
                radius: 2
                color: AppTheme.outlineStrong
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
