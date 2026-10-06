import QtQuick
import QtQuick.Layouts
import "../Controls" as Ctrl
import "../Singletons" as S
import "../Drawers" as D
import com.topicdev.reroll 1.0
Item{
    id: root
    Flickable {
        anchors.fill:  parent
        contentWidth:  width
        contentHeight: _col.implicitHeight
        clip:          true
        Column {
            id: _col
            width:         parent.width
            topPadding:    24
            bottomPadding: 24
            spacing:       15

            Text {
                x: 16
                width: parent.width - 32
                text: qsTr("SETTINGS")
                color: S.AppTheme.textPrimary
                font.pixelSize: S.AppTheme.fs28
                font.weight: Font.Black
                elide: Text.ElideRight
            }

            Ctrl.ThemePicker {
                width: parent.width
            }
            Ctrl.AccentPicker {
                width: parent.width
            }
            Ctrl.FontSizePicker{
                width: parent.width
            }

            Rectangle {
                x: 16
                width: parent.width - 32
                height: 1
                color: S.AppTheme.outline
            }

            Column {
                x: 16
                width: parent.width - 32
                spacing: S.AppTheme.spacing4

                Text {
                    text: qsTr("HELP")
                    color: S.AppTheme.textSecondary
                    font.pixelSize: S.AppTheme.fs12
                    font.weight: Font.Black
                    font.letterSpacing: 1
                }

                Ctrl.SettingsLinkRow {
                    objectName: "settingsHowRerollWorksRow"
                    width: parent.width
                    text: qsTr("How Reroll works")
                    onClicked: _rerollTips.open()
                }

                Ctrl.SettingsLinkRow {
                    objectName: "settingsHowDiscoverWorksRow"
                    width: parent.width
                    text: qsTr("How Discover works")
                    onClicked: _discoverTips.open()
                }

                Ctrl.SettingsLinkRow {
                    objectName: "settingsHowMyListWorksRow"
                    width: parent.width
                    text: qsTr("How My List works")
                    onClicked: _myListTips.open()
                }
            }

            Rectangle {
                x: 16
                width: parent.width - 32
                height: 1
                color: S.AppTheme.outline
            }

            Column {
                x: 16
                width: parent.width - 32
                spacing: S.AppTheme.spacing12

                Text {
                    text: qsTr("CREDITS")
                    color: S.AppTheme.textSecondary
                    font.pixelSize: S.AppTheme.fs12
                    font.weight: Font.Black
                    font.letterSpacing: 1
                }

                Ctrl.TmdbAttribution {
                    objectName: "settingsTmdbAttribution"
                    width: parent.width
                    compact: false
                }
            }

            Rectangle {
                x: 16
                width: parent.width - 32
                height: 1
                color: S.AppTheme.outline
            }

            Column {
                x: 16
                width: parent.width - 32
                spacing: S.AppTheme.spacing4

                Text {
                    text: qsTr("SUPPORT")
                    color: S.AppTheme.textSecondary
                    font.pixelSize: S.AppTheme.fs12
                    font.weight: Font.Black
                    font.letterSpacing: 1
                }

                Ctrl.SettingsLinkRow {
                    objectName: "settingsRateAppRow"
                    width: parent.width
                    text: qsTr("Rate this app")
                    showStars: true
                    url: Qt.platform.os === "android"
                         ? "market://details?id=com.topicdev.reroll"
                         : "https://play.google.com/store/apps/details?id=com.topicdev.reroll"
                }
            }

            Rectangle {
                x: 16
                width: parent.width - 32
                height: 1
                color: S.AppTheme.outline
            }

            Column {
                x: 16
                width: parent.width - 32
                spacing: S.AppTheme.spacing4

                Text {
                    text: qsTr("LEGAL")
                    color: S.AppTheme.textSecondary
                    font.pixelSize: S.AppTheme.fs12
                    font.weight: Font.Black
                    font.letterSpacing: 1
                }

                Ctrl.SettingsLinkRow {
                    objectName: "settingsTermsOfUseRow"
                    width: parent.width
                    text: qsTr("Terms of Use")
                    url: "https://topicdev.com/reroll/terms.html"
                }

                Ctrl.SettingsLinkRow {
                    objectName: "settingsPrivacyPolicyRow"
                    width: parent.width
                    text: qsTr("Privacy Policy")
                    url: "https://topicdev.com/reroll/privacy.html"
                }
            }

            Rectangle {
                x: 16
                width: parent.width - 32
                height: 1
                color: S.AppTheme.outline
            }

            Column {
                x: 16
                width: parent.width - 32
                spacing: S.AppTheme.spacing10

                Text {
                    text: qsTr("MORE FROM TOPICDEV")
                    color: S.AppTheme.textSecondary
                    font.pixelSize: S.AppTheme.fs12
                    font.weight: Font.Black
                    font.letterSpacing: 1
                }

                Ctrl.MakimediaPromoCard {
                    objectName: "settingsMakimediaCard"
                    width: parent.width
                }
            }

            Text {
                objectName: "settingsAppVersion"
                x: 16
                width: parent.width - 32
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Reroll v%1").arg(AppSettings.appVersion)
                color: S.AppTheme.textSecondary
                font.pixelSize: S.AppTheme.fs14
                font.weight: Font.Medium
            }
        }
    }

    D.TutorialSheet {
        id: _rerollTips
        objectName: "settingsRerollTutorialSheet"

        heading: qsTr("How Reroll Works")
        sections: [
            {
                heading: qsTr("Reroll"),
                body: qsTr("Tap the big Reroll button to get a new suggestion. Don't like it? Reroll again.")
            },
            {
                heading: qsTr("Swipe"),
                body: qsTr("Swipe left to reroll, swipe right to go back to the previous suggestion, and tap or swipe up for details.")
            },
            {
                heading: qsTr("Filters"),
                body: qsTr("Tap Filters to narrow results by media type, genre, year, and rating.")
            },
            {
                heading: qsTr("Watchlist & watched"),
                body: qsTr("Save a title for later or mark it watched right from the action bar. The X hides a title for good.")
            }
        ]
    }

    D.TutorialSheet {
        id: _discoverTips
        objectName: "settingsDiscoverTutorialSheet"

        heading: qsTr("How Discover Works")
        sections: [
            {
                heading: qsTr("Search"),
                body: qsTr("Search for a specific movie or TV show by title.")
            },
            {
                heading: qsTr("Browse"),
                body: qsTr("Scroll down to browse trending, popular, and titles by genre.")
            },
            {
                heading: qsTr("Quick actions"),
                body: qsTr("Tap the bookmark or checkmark badge on any poster to save it to your watchlist or mark it watched. Tap the poster itself for more details.")
            }
        ]
    }

    D.TutorialSheet {
        id: _myListTips
        objectName: "settingsMyListTutorialSheet"

        heading: qsTr("How My List Works")
        sections: [
            {
                heading: qsTr("Details"),
                body: qsTr("Tap a title to see its details, where you can also add it to your watchlist or mark it watched.")
            },
            {
                heading: qsTr("More options"),
                body: qsTr("Long-press a title to move it between watchlist and watched, or remove it from your list.")
            },
            {
                heading: qsTr("Undo"),
                body: qsTr("Changed something by accident? Tap Undo on the message at the bottom of the screen.")
            },
            {
                heading: qsTr("Hidden titles"),
                body: qsTr("Titles you hide on Reroll live in the Hidden tab. Tap Restore to bring one back into suggestions.")
            }
        ]
    }
}
