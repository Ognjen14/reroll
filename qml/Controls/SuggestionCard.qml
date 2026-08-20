pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons"

Item {
    id: root

    enum MediaType {
        NoMedia,
        Movie,
        Tv
    }

    property string title
    property int year: 0
    property int mediaType: SuggestionCard.NoMedia
    property double rating: 0.0
    property string overview
    property var genreNames: []
    property var streamingProviders: []
    property bool wideLayout: false
    property bool recycled: false
    property real cardPadding: wideLayout ? AppTheme.spacing24 : AppTheme.spacing18

    readonly property color chipBackgroundColor: Qt.rgba(1, 1, 1, 0.16)
    readonly property color overlayTextColor: "white"
    readonly property color overlaySecondaryTextColor: Qt.rgba(1, 1, 1, 0.75)

    readonly property string mediaTypeText: {
        switch (mediaType) {
        case SuggestionCard.Movie:
            return qsTr("Movie")
        case SuggestionCard.Tv:
            return qsTr("TV")
        default:
            return ""
        }
    }
    readonly property string yearText: year > 0 ? year.toString() : ""
    readonly property string ratingText: rating > 0 ? rating.toFixed(1) : ""
    readonly property string metaText: [mediaTypeText, yearText]
                                       .filter(value => value.length > 0)
                                       .join("  ·  ")

    implicitWidth: wideLayout ? 900 : 360
    implicitHeight: _content.implicitHeight
    height: implicitHeight

    ColumnLayout {
        id: _content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: root.cardPadding
        spacing: AppTheme.spacing8

        Text {
            Layout.fillWidth: true
            visible: root.recycled
            wrapMode: Text.WordWrap
            text: qsTr("You've seen everything that matches - starting over.")
            color: AppTheme.warning
            font.pixelSize: AppTheme.fs12
            font.weight: Font.DemiBold
        }
        TmdbAttribution {
            objectName: "suggestionTmdbAttribution"

            compact: true
            logoHeight: 12
        }
        Flow {
            objectName: "suggestionMetaFlow"

            Layout.fillWidth: true
            spacing: AppTheme.spacing8

            GenreTag {
                visible: root.ratingText.length > 0
                text: root.ratingText
                iconSource: "qrc:/assets/start_rating.png"
                backgroundColor: root.chipBackgroundColor
                foregroundColor: root.overlayTextColor
                borderColor: "transparent"
            }

            GenreTag {
                visible: root.metaText.length > 0
                text: root.metaText
                backgroundColor: root.chipBackgroundColor
                foregroundColor: root.overlayTextColor
                borderColor: "transparent"
            }

            Repeater {
                id: repeater

                model: root.genreNames

                delegate: GenreTag {
                    required property string modelData

                    text: modelData
                    backgroundColor: root.chipBackgroundColor
                    foregroundColor: root.overlayTextColor
                    borderColor: "transparent"
                }
            }

        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: AppTheme.spacing4
            text: root.title.length > 0 ? root.title : qsTr("No suggestion")
            color: root.overlayTextColor
            font.pixelSize: root.wideLayout ? AppTheme.fs28 : AppTheme.fs22
            font.weight: Font.Black
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        Text {
            Layout.fillWidth: true
            text: root.overview.length > 0
                  ? root.overview
                  : qsTr("No overview is available for this title.")
            color: root.overlaySecondaryTextColor
            font.pixelSize: root.wideLayout ? AppTheme.fs14 : AppTheme.fs12
            lineHeight: 1.4
            wrapMode: Text.Wrap
            maximumLineCount: root.wideLayout ? 4 : 3
            elide: Text.ElideRight
        }

        Flow {
            objectName: "suggestionStreamingProvidersFlow"

            Layout.fillWidth: true
            Layout.topMargin: AppTheme.spacing4
            spacing: AppTheme.spacing6
            visible: root.streamingProviders.length > 0

            Repeater {
                model: root.streamingProviders

                delegate: Rectangle {
                    required property var modelData

                    width: 24
                    height: 24
                    radius: AppTheme.radiusSmall
                    color: root.chipBackgroundColor

                    Image {
                        anchors.fill: parent
                        anchors.margins: 3
                        source: modelData.logoUrl
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        smooth: true
                    }
                }
            }
        }
    }
}
