pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import QtQuick.Layouts
import "../Controls"
import "../Singletons"
import com.topicdev.reroll 1.0

Basic.Drawer {
    id: root

    property int minimumSelectableYear: 1900
    property int maximumSelectableYear: 2026

    edge: Qt.BottomEdge
    implicitWidth: 420
    implicitHeight: 640
    width: parent ? parent.width : implicitWidth
    height: parent
            ? Math.min(parent.height * 0.88, implicitHeight)
            : implicitHeight
    modal: true
    dim: true
    closePolicy: Basic.Popup.CloseOnEscape
                 | Basic.Popup.CloseOnPressOutside

    function synchronizeInputs() {
        _mediaType.selectedMediaType = HomeController.editableMediaType
        _yearRange.setRangeSilently(HomeController.editableMinimumYear,
                                    HomeController.editableMaximumYear)
        _minimumRating.setValueSilently(HomeController.editableMinimumRating)
        _genreMatchMode.selectedMode = HomeController.editableGenreMatchMode
        _excludeWatched.checked = HomeController.editableExcludeWatched
    }

    function commitValidInputs() {
        const minimumYear = _yearRange.effectiveMinimum
        const maximumYear = _yearRange.effectiveMaximum
        const minimumRating = _minimumRating.value
        HomeController.setEditableYearRange(minimumYear, maximumYear)
        HomeController.setEditableMinimumRating(minimumRating)
    }

    function applyFilters() {
        commitValidInputs()
        const applied = HomeController.apply()
        root.close()
        return applied
    }

    function resetFilters() {
        const reset = HomeController.reset()
        synchronizeInputs()
        return reset
    }

    function closeDrawer() {
        commitValidInputs()
        root.close()
    }

    onOpened: {
        synchronizeInputs()
        PopupRegistry.register(root)
    }
    onClosed: PopupRegistry.unregister(root)
    Component.onCompleted: synchronizeInputs()

    Connections {
        target: HomeController

        function onEditableFiltersChanged() {
            root.synchronizeInputs()
        }
    }

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
                objectName: "filterDrawerHandle"

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

            objectName: "filterDrawerScrollView"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            Column {
                property real horizontalInset: AppTheme.spacing18

                width: _scrollView.availableWidth
                spacing: AppTheme.spacing14

                Text {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    text: qsTr("Filters")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fs18
                    font.weight: Font.Bold
                }

                UnappliedChangesNotice {
                    objectName: "filterUnappliedChangesNotice"
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    active: HomeController.hasUnappliedChanges
                }

                MediaTypeSelector {
                    id: _mediaType

                    objectName: "filterMediaTypeSelector"
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset

                    onMediaTypeActivated: function(mediaType) {
                        HomeController.setEditableMediaType(mediaType)
                    }
                }

                RangeSliderInput {
                    id: _yearRange

                    objectName: "filterYearRangeInput"
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    labelText: qsTr("Year")
                    from: root.minimumSelectableYear
                    to: root.maximumSelectableYear

                    onRangeCommitted: function(minimumYear, maximumYear) {
                        HomeController.setEditableYearRange(minimumYear, maximumYear)
                    }
                }

                SliderInput {
                    id: _minimumRating

                    objectName: "filterMinimumRatingInput"
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    labelText: qsTr("Minimum rating")
                    from: 0
                    to: 10
                    stepSize: 0.5
                    decimals: 1

                    onValueCommitted: function(value) {
                        HomeController.setEditableMinimumRating(value)
                    }
                }

                Column {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    spacing: 10

                    Text {
                        width: parent.width
                        text: qsTr("Genres")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fs13
                        font.weight: Font.Medium
                    }

                    Flow {
                        id: _genreFlow

                        objectName: "filterGenreFlow"
                        width: parent.width
                        spacing: 8

                        Repeater {
                            id: _genreRepeater

                            model: HomeController.genreModel

                            delegate: GenreChipDelegate {
                                objectName: "filterGenreOption" + genreId

                                onSelectionRequested: function(genreId, selected) {
                                    HomeController.setGenreSelected(genreId, selected)
                                }
                            }
                        }
                    }
                }

                GenreMatchModeSelector {
                    id: _genreMatchMode

                    objectName: "filterGenreMatchModeSelector"
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset

                    onModeActivated: function(mode) {
                        HomeController.setEditableGenreMatchMode(mode)
                    }
                }

                RowLayout {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    spacing: AppTheme.spacing12

                    Column {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            width: parent.width
                            text: qsTr("Hide watched titles")
                            color: AppTheme.textPrimary
                            font.pixelSize: AppTheme.fs14
                            font.weight: Font.DemiBold
                        }

                        Text {
                            width: parent.width
                            text: qsTr("Don't suggest anything already marked as watched")
                            color: AppTheme.textSecondary
                            font.pixelSize: AppTheme.fs11
                            wrapMode: Text.Wrap
                        }
                    }

                    ToggleSwitch {
                        id: _excludeWatched

                        objectName: "filterExcludeWatchedToggle"
                        text: qsTr("Hide titles I've already watched")

                        onToggled: HomeController.setEditableExcludeWatched(checked)
                    }
                }

                Row {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    spacing: 8

                    AppButton {
                        objectName: "filterCloseButton"
                        width: (parent.width - parent.spacing) / 2
                        height: AppTheme.controlHeightMedium
                        text: qsTr("Close")
                        accessibleName: qsTr("Close filters")
                        contentRadius: AppTheme.radiusLarge
                        backgroundColor: AppTheme.surfaceVariant
                        foregroundColor: AppTheme.textSecondary
                        borderColor: "transparent"

                        onClicked: root.closeDrawer()
                    }

                    AppButton {
                        objectName: "filterResetButton"
                        width: (parent.width - parent.spacing) / 2
                        height: AppTheme.controlHeightMedium
                        text: qsTr("Reset")
                        accessibleName: qsTr("Reset filter draft")
                        contentRadius: AppTheme.radiusLarge
                        backgroundColor: AppTheme.surfaceVariant
                        foregroundColor: AppTheme.textSecondary
                        borderColor: "transparent"

                        onClicked: root.resetFilters()
                    }
                }

                AppButton {
                    objectName: "filterApplyButton"
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    height: AppTheme.controlHeightLarge
                    text: qsTr("Apply Filters")
                    accessibleName: qsTr("Apply filters")
                    contentRadius: AppTheme.radiusPill
                    foregroundColor: AppTheme.darkMode ? AppTheme.onPrimary : "#FFFFFF"

                    onClicked: root.applyFilters()
                }

                Item {
                    width: 1
                    height: 20
                }
            }
        }
    }
}
