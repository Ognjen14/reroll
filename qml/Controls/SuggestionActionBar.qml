pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons"

RowLayout {
    id: root

    property bool actionsEnabled: true
    property bool watchlist: false
    property bool watched: false
    property bool trailerLoading: false
    property bool rerollEnabled: true
    property bool rerollBusy: false

    readonly property int actionButtonSize: AppTheme.touchTargetMinimum
    readonly property int rerollButtonSize: Math.round(actionButtonSize * 1.3)

    signal watchlistToggleRequested()
    signal watchedToggleRequested()
    signal trailerRequested()
    signal rerollRequested()
    signal hideRequested()

    spacing: AppTheme.spacing20

    AppButton {
        objectName: "suggestionWatchlistButton"

        shape: AppButton.CircleShape
        shapeSize: root.actionButtonSize
        imageSource: root.watchlist
                     ? "qrc:/assets/reroll_page/whichlisted.png"
                     : "qrc:/assets/reroll_page/whichlist.png"
        imageSize: 22
        accessibleName: root.watchlist
                        ? qsTr("Remove from watchlist")
                        : qsTr("Add to watchlist")
        enabled: root.actionsEnabled
        backgroundColor: root.watchlist
                         ? AppTheme.infoContainer
                         : AppTheme.surfaceVariant
        foregroundColor: root.watchlist
                         ? AppTheme.info
                         : AppTheme.textPrimary
        borderColor: root.watchlist ? AppTheme.info : AppTheme.outline

        Accessible.checked: root.watchlist

        onClicked: root.watchlistToggleRequested()
    }

    AppButton {
        objectName: "suggestionTrailerButton"

        shape: AppButton.CircleShape
        shapeSize: root.actionButtonSize
        imageSource: "qrc:/assets/reroll_page/trailler_play.svg"
        imageSize: 22
        accessibleName: qsTr("Watch trailer")
        enabled: root.actionsEnabled && !root.trailerLoading
        backgroundColor: AppTheme.surfaceVariant
        foregroundColor: AppTheme.textPrimary
        borderColor: AppTheme.outline

        onClicked: root.trailerRequested()
    }

    RerollButton {
        id: _rerollButton
        objectName: "suggestionRerollButton"

        shape: AppButton.CircleShape
        shapeSize: root.rerollButtonSize
        imageSource: "qrc:/assets/reroll_page/reroll.png"
        imageSize: Math.round(22 * 1.3)
        accessibleName: qsTr("Reroll suggestion")
        enabled: root.rerollEnabled
        busy: root.rerollBusy
        foregroundColor: AppTheme.onPrimaryIcon

        onClicked: {
            spin()
            root.rerollRequested()
        }
    }

    AppButton {
        objectName: "suggestionWatchedButton"

        shape: AppButton.CircleShape
        shapeSize: root.actionButtonSize
        imageSource: root.watched
                     ? "qrc:/assets/reroll_page/marked_watched.png"
                     : "qrc:/assets/reroll_page/mark_watched.png"
        imageSize: 22
        accessibleName: root.watched
                        ? qsTr("Unmark as watched")
                        : qsTr("Mark as watched")
        enabled: root.actionsEnabled
        backgroundColor: root.watched
                         ? AppTheme.successContainer
                         : AppTheme.surfaceVariant
        foregroundColor: root.watched
                         ? AppTheme.success
                         : AppTheme.textPrimary
        borderColor: root.watched ? AppTheme.success : AppTheme.outline

        Accessible.checked: root.watched

        onClicked: root.watchedToggleRequested()
    }

    AppButton {
        objectName: "suggestionHideButton"

        shape: AppButton.CircleShape
        shapeSize: root.actionButtonSize
        imageSource: "qrc:/assets/reroll_page/x.png"
        imageSize: 22
        accessibleName: qsTr("Don't show this title again")
        enabled: root.actionsEnabled
        backgroundColor: AppTheme.surfaceVariant
        foregroundColor: AppTheme.textPrimary
        borderColor: AppTheme.outline

        onClicked: root.hideRequested()
    }
}
