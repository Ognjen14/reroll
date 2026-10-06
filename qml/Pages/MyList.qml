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

    readonly property int modeAll: 0
    readonly property int modeWatchlist: 1
    readonly property int modeWatched: 2
    readonly property int modeMovies: 3
    readonly property int modeTv: 4
    readonly property int modeHidden: 5

    readonly property int currentMode: MyListController.filteredModel.mode
    readonly property bool hiddenTab: currentMode === modeHidden

    readonly property var filterOptions: [
        { label: qsTr("All"), value: modeAll },
        { label: qsTr("Watchlist"), value: modeWatchlist },
        { label: qsTr("Watched"), value: modeWatched },
        { label: qsTr("Movies"), value: modeMovies },
        { label: qsTr("TV Shows"), value: modeTv },
        { label: qsTr("Hidden"), value: modeHidden }
    ]

    readonly property var sortOptions: [
        { label: qsTr("Date added"), value: 0 },
        { label: qsTr("Title"), value: 1 },
        { label: qsTr("Release year"), value: 2 },
        { label: qsTr("Rating"), value: 3 }
    ]

    readonly property string currentSortLabel: {
        const sortMode = MyListController.filteredModel.sortMode
        for (let i = 0; i < sortOptions.length; ++i) {
            if (sortOptions[i].value === sortMode)
                return sortOptions[i].label
        }
        return sortOptions[0].label
    }

    readonly property int currentModeCount: {
        MyListController.revision
        return MyListController.countForMode(root.currentMode)
    }

    readonly property var emptyState: {
        MyListController.revision
        const listEmpty = MyListController.totalCount() === 0
        switch (root.currentMode) {
        case root.modeWatchlist:
            return {
                title: qsTr("Nothing on your watchlist"),
                message: qsTr("Tap the bookmark on a suggestion or poster to save it for later."),
                action: qsTr("Get a suggestion"),
                tab: 0
            }
        case root.modeWatched:
            return {
                title: qsTr("Nothing marked as watched"),
                message: qsTr("Mark titles you've already seen so you can keep track of them and skip them in suggestions."),
                action: qsTr("Get a suggestion"),
                tab: 0
            }
        case root.modeMovies:
            return {
                title: qsTr("No movies in your list"),
                message: qsTr("Movies you save or mark as watched will show up here."),
                action: qsTr("Browse Discover"),
                tab: 1
            }
        case root.modeTv:
            return {
                title: qsTr("No TV shows in your list"),
                message: qsTr("TV shows you save or mark as watched will show up here."),
                action: qsTr("Browse Discover"),
                tab: 1
            }
        case root.modeHidden:
            return {
                title: qsTr("No hidden titles"),
                message: qsTr("Titles you hide with the X on Reroll show up here, so you can bring them back any time."),
                action: "",
                tab: -1
            }
        default:
            return {
                title: listEmpty ? qsTr("Your list is empty") : qsTr("Nothing here yet"),
                message: qsTr("Save titles to your watchlist or mark them as watched from Reroll or Discover, and they'll show up here."),
                action: qsTr("Get a suggestion"),
                tab: 0
            }
        }
    }

    function goToTab(index) {
        const appWindow = ApplicationWindow.window
        if (appWindow && appWindow.switchTab)
            appWindow.switchTab(index)
    }

    function setSortMode(sortMode) {
        MyListController.filteredModel.sortMode = sortMode
        AppSettings.myListSortMode = sortMode
    }

    function openSortSheet() {
        const currentSort = MyListController.filteredModel.sortMode
        _actionSheet.openWith(qsTr("Sort by"), "", root.sortOptions.map(function(option) {
            return {
                label: option.label,
                selected: option.value === currentSort,
                handler: function() { root.setSortMode(option.value) }
            }
        }))
    }

    function applyFlags(item, watchlist, watched, hidden) {
        const changes = []
        if (item.watchlist !== watchlist)
            changes.push({ setter: "setWatchlist", value: watchlist })
        if (item.watched !== watched)
            changes.push({ setter: "setWatched", value: watched })
        if (item.hidden !== hidden)
            changes.push({ setter: "setHidden", value: hidden })

        changes.sort(function(left, right) { return (right.value ? 1 : 0) - (left.value ? 1 : 0) })
        for (let i = 0; i < changes.length; ++i) {
            MyListController[changes[i].setter](item.tmdbId, item.mediaType, item.title,
                                                item.releaseYear, item.genreIds, item.posterPath,
                                                item.rating, item.voteCount, changes[i].value)
        }
    }

    function changeWithUndo(item, watchlist, watched, hidden, message) {
        const undoState = MyListController.captureState(item.tmdbId, item.mediaType)
        root.applyFlags(item, watchlist, watched, hidden)
        _toast.show(message, qsTr("Undo"), function() {
            MyListController.restoreState(undoState)
        })
    }

    function openDetails(item) {
        _titleDetailsDrawer.openFor(item.tmdbId, item.mediaType, item.title,
                                    item.releaseYear, item.posterPath, item.rating,
                                    item.genreIds, item.voteCount)
    }

    function statusText(item) {
        const parts = [item.mediaType === 1 ? qsTr("TV") : qsTr("Movie")]
        if (item.releaseYear > 0)
            parts.push(item.releaseYear.toString())
        if (item.hidden && root.hiddenTab)
            parts.push(qsTr("Hidden from suggestions"))
        else if (item.watchlist && item.watched)
            parts.push(qsTr("On watchlist and watched"))
        else if (item.watchlist)
            parts.push(qsTr("On your watchlist"))
        else if (item.watched)
            parts.push(qsTr("Watched"))
        return parts.join("  ·  ")
    }

    function openActions(item) {
        const actions = [
            { label: qsTr("View details"), handler: function() { root.openDetails(item) } }
        ]

        if (root.hiddenTab) {
            actions.push({
                label: qsTr("Show in suggestions again"),
                handler: function() {
                    root.changeWithUndo(item, item.watchlist, item.watched, false,
                                        qsTr("Will show in suggestions again"))
                }
            })
            _actionSheet.openWith(item.title, root.statusText(item), actions)
            return
        }

        if (item.watchlist && !item.watched) {
            actions.push({
                label: qsTr("Mark as watched"),
                handler: function() {
                    root.changeWithUndo(item, false, true, item.hidden, qsTr("Moved to watched"))
                }
            })
        }
        if (item.watched && !item.watchlist) {
            actions.push({
                label: qsTr("Move back to watchlist"),
                handler: function() {
                    root.changeWithUndo(item, true, false, item.hidden, qsTr("Moved to watchlist"))
                }
            })
        }
        if (item.watchlist) {
            actions.push({
                label: qsTr("Remove from watchlist"),
                destructive: true,
                handler: function() {
                    root.changeWithUndo(item, false, item.watched, item.hidden,
                                        qsTr("Removed from watchlist"))
                }
            })
        }
        if (item.watched) {
            actions.push({
                label: qsTr("Remove from watched"),
                destructive: true,
                handler: function() {
                    root.changeWithUndo(item, item.watchlist, false, item.hidden,
                                        qsTr("Removed from watched"))
                }
            })
        }
        if (item.watchlist && item.watched) {
            actions.push({
                label: qsTr("Remove from My List"),
                destructive: true,
                handler: function() {
                    root.changeWithUndo(item, false, false, item.hidden,
                                        qsTr("Removed from My List"))
                }
            })
        }

        _actionSheet.openWith(item.title, root.statusText(item), actions)
    }

    Component.onCompleted: MyListController.filteredModel.sortMode = AppSettings.myListSortMode

    Rectangle {
        anchors.fill: parent
        color: S.AppTheme.background
    }

    TitleDetailsDrawer {
        id: _titleDetailsDrawer
        objectName: "myListTitleDetailsDrawer"
    }

    ActionSheet {
        id: _actionSheet
        objectName: "myListActionSheet"
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

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: -S.AppTheme.spacing8
            spacing: S.AppTheme.spacing8

            Text {
                Layout.fillWidth: true
                text: qsTr("%n title(s)", "", root.currentModeCount)
                color: S.AppTheme.textSecondary
                font.pixelSize: S.AppTheme.fs14
                elide: Text.ElideRight
            }

            AppButton {
                objectName: "myListSortButton"

                visible: root.currentModeCount > 1
                text: qsTr("Sort: %1").arg(root.currentSortLabel)
                accessibleName: qsTr("Sort by %1. Change sort order").arg(root.currentSortLabel)
                contentRadius: S.AppTheme.radiusPill
                backgroundColor: S.AppTheme.surfaceVariant
                foregroundColor: S.AppTheme.textPrimary
                borderColor: S.AppTheme.outline

                onClicked: root.openSortSheet()
            }
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

                readonly property bool active: root.currentMode === modelData.value
                readonly property int count: {
                    MyListController.revision
                    return MyListController.countForMode(modelData.value)
                }

                objectName: "myListFilterChip" + modelData.value
                text: count > 0 ? qsTr("%1  %2").arg(modelData.label).arg(count) : modelData.label
                accessibleName: qsTr("%1, %n title(s)", "", count).arg(modelData.label)
                contentRadius: S.AppTheme.radiusPill
                backgroundColor: active ? S.AppTheme.primary : S.AppTheme.surfaceVariant
                foregroundColor: active
                                 ? (S.AppTheme.darkMode ? S.AppTheme.onPrimary : "#FFFFFF")
                                 : S.AppTheme.textPrimary
                borderColor: active ? "transparent" : S.AppTheme.outline

                onClicked: MyListController.filteredModel.mode = modelData.value
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: _grid.count === 0

            StatePanel {
                objectName: "myListEmptyState"

                anchors.centerIn: parent
                width: Math.min(parent.width, 420)
                mode: StatePanel.Empty
                iconOverride: "qrc:/assets/nav/my_list.svg"
                titleText: root.emptyState.title
                messageText: root.emptyState.message
                retryVisible: root.emptyState.tab >= 0
                retryText: root.emptyState.action
                secondaryVisible: root.currentMode === root.modeAll
                                  || root.currentMode === root.modeWatchlist
                secondaryText: qsTr("Browse Discover")

                onRetryRequested: root.goToTab(root.emptyState.tab)
                onSecondaryRequested: root.goToTab(1)
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
            cellHeight: cellWidth * S.AppTheme.posterAspectRatio + 52
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
                required property bool hidden

                readonly property bool canMoveToWatched: !root.hiddenTab && watchlist && !watched
                readonly property bool canMoveToWatchlist: !root.hiddenTab && watched && !watchlist

                function snapshot() {
                    return {
                        tmdbId: _card.tmdbId,
                        mediaType: _card.mediaType,
                        title: _card.title,
                        releaseYear: _card.releaseYear,
                        genreIds: _card.genreIds,
                        posterPath: _card.posterPath,
                        rating: _card.rating,
                        voteCount: _card.voteCount,
                        watchlist: _card.watchlist,
                        watched: _card.watched,
                        hidden: _card.hidden
                    }
                }

                width: _grid.cellWidth
                height: _grid.cellHeight

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: S.AppTheme.spacing6
                    spacing: S.AppTheme.spacing4

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: width * S.AppTheme.posterAspectRatio
                        scale: _posterArea.pressed ? 0.97 : 1.0

                        Behavior on scale {
                            NumberAnimation { duration: 120 }
                        }

                        PosterImage {
                            anchors.fill: parent
                            opacity: root.hiddenTab ? 0.55 : 1.0
                            source: PosterUrlResolver.resolveUrl(_card.posterPath, 342)
                        }

                        MouseArea {
                            id: _posterArea
                            objectName: "myListPosterArea"

                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            pressAndHoldInterval: 450
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            Accessible.role: Accessible.Button
                            Accessible.name: _card.title

                            onClicked: function(mouse) {
                                if (mouse.button === Qt.RightButton)
                                    root.openActions(_card.snapshot())
                                else
                                    root.openDetails(_card.snapshot())
                            }
                            onPressAndHold: root.openActions(_card.snapshot())
                        }

                        Rectangle {
                            objectName: "myListWatchlistBadge"

                            visible: !root.hiddenTab && _card.watchlist
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.margins: S.AppTheme.spacing6
                            width: 24
                            height: 24
                            radius: 12
                            color: S.AppTheme.info

                            Accessible.role: Accessible.StaticText
                            Accessible.name: qsTr("On watchlist")

                            ThemedIcon {
                                anchors.centerIn: parent
                                width: 13
                                height: 13
                                source: "qrc:/assets/reroll_page/whichlisted.png"
                                tintColor: "white"
                            }
                        }

                        Rectangle {
                            objectName: "myListWatchedBadge"

                            visible: !root.hiddenTab && _card.watched
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: S.AppTheme.spacing6
                            width: 24
                            height: 24
                            radius: 12
                            color: S.AppTheme.success

                            Accessible.role: Accessible.StaticText
                            Accessible.name: qsTr("Watched")

                            ThemedIcon {
                                anchors.centerIn: parent
                                width: 13
                                height: 13
                                source: "qrc:/assets/reroll_page/marked_watched.png"
                                tintColor: "white"
                            }
                        }

                        Rectangle {
                            objectName: "myListQuickActionBar"

                            visible: _card.canMoveToWatched || _card.canMoveToWatchlist || root.hiddenTab
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 30
                            color: _quickActionArea.pressed ? Qt.rgba(0, 0, 0, 0.85) : Qt.rgba(0, 0, 0, 0.7)

                            Accessible.role: Accessible.Button
                            Accessible.name: _quickActionText.text

                            Text {
                                id: _quickActionText

                                anchors.centerIn: parent
                                width: parent.width - 2 * S.AppTheme.spacing6
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                                text: root.hiddenTab
                                      ? qsTr("Restore")
                                      : (_card.canMoveToWatched
                                         ? qsTr("Mark as watched")
                                         : qsTr("Back to watchlist"))
                                color: "white"
                                font.pixelSize: S.AppTheme.fs11
                                font.weight: Font.DemiBold
                            }

                            MouseArea {
                                id: _quickActionArea

                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    const item = _card.snapshot()
                                    if (root.hiddenTab)
                                        root.changeWithUndo(item, item.watchlist, item.watched, false,
                                                            qsTr("Will show in suggestions again"))
                                    else if (_card.canMoveToWatched)
                                        root.changeWithUndo(item, false, true, item.hidden,
                                                            qsTr("Moved to watched"))
                                    else
                                        root.changeWithUndo(item, true, false, item.hidden,
                                                            qsTr("Moved to watchlist"))
                                }
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: _card.title
                        color: root.hiddenTab ? S.AppTheme.textSecondary : S.AppTheme.textPrimary
                        font.pixelSize: S.AppTheme.fs13
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                        maximumLineCount: 1
                    }

                    Text {
                        Layout.fillWidth: true
                        text: {
                            const parts = []
                            if (_card.releaseYear > 0)
                                parts.push(_card.releaseYear.toString())
                            if (_card.rating > 0)
                                parts.push(_card.rating.toFixed(1))
                            return parts.join("  ·  ")
                        }
                        visible: text.length > 0
                        color: S.AppTheme.textSecondary
                        font.pixelSize: S.AppTheme.fs11
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
