import QtQuick
import QtQuick.Layouts
import "../Controls" as Ctrl
import "../Singletons" as S
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
                    url: "https://sites.google.com/view/rerollapp/terms-of-use"
                }

                Ctrl.SettingsLinkRow {
                    objectName: "settingsPrivacyPolicyRow"
                    width: parent.width
                    text: qsTr("Privacy Policy")
                    url: "https://sites.google.com/view/rerollapp/privacy-policy"
                }
            }
        }
    }
}
