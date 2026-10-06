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

    readonly property string applyText: {
        if (HomeController.matchCountLoading)
            return qsTr("Apply · counting...")
        const count = HomeController.editableMatchCount
        if (count <= 0)
            return qsTr("Apply Filters")
        if (count === 1)
            return qsTr("Apply · 1 title")
        return qsTr("Apply · %1 titles").arg(Number(count).toLocaleString(Qt.locale(), "f", 0))
    }


    edge: Qt.BottomEdge
    implicitWidth: 420
    implicitHeight: 640
    width: parent ? parent.width : implicitWidth
    height: parent
            ? Math.min(parent.height * 0.88, implicitHeight)
            : implicitHeight
    modal: true
    dim: true
    interactive: false
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
        HomeController.refreshMatchCount()
        HomeController.ensureGenreLists()
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
            Layout.preferredHeight: 44

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

            AppButton {
                objectName: "filterDrawerCloseButton"

                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: AppTheme.spacing12
                anchors.topMargin: AppTheme.spacing4
                shape: AppButton.CircleShape
                shapeSize: 32
                imageSource: "qrc:/assets/reroll_page/x.png"
                imageSize: 14
                backgroundColor: AppTheme.surfaceVariant
                foregroundColor: AppTheme.textSecondary
                accessibleName: qsTr("Close filters")

                onClicked: root.closeDrawer()
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

                RowLayout {
                    x: parent.horizontalInset
                    width: parent.width - 2 * parent.horizontalInset
                    spacing: AppTheme.spacing12

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Filters")
                        color: AppTheme.textPrimary
                        font.pixelSize: AppTheme.fs18
                        font.weight: Font.Bold
                    }

                    Text {
                        objectName: "filterResetButton"

                        text: qsTr("Reset")
                        color: AppTheme.primary
                        font.pixelSize: AppTheme.fs14
                        font.weight: Font.DemiBold
                        opacity: _resetArea.pressed ? 0.6 : 1.0

                        Accessible.role: Accessible.Button
                        Accessible.name: qsTr("Reset filter draft")

                        MouseArea {
                            id: _resetArea

                            anchors.fill: parent
                            anchors.margins: -AppTheme.spacing12
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.resetFilters()
                        }
                    }
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

                Item {
                    width: 1
                    height: AppTheme.spacing8
                }
            }
        }

        Rectangle {
            objectName: "filterApplyBar"

            Layout.fillWidth: true
            Layout.preferredHeight: _noMatchesText.height + _applyButton.height
                                    + AppTheme.spacing12 + AppTheme.spacing20
            color: AppTheme.surface

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: AppTheme.outline
            }

            Text {
                id: _noMatchesText
                objectName: "filterNoMatchesText"

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.leftMargin: AppTheme.spacing18
                anchors.rightMargin: AppTheme.spacing18
                anchors.topMargin: visible ? AppTheme.spacing10 : 0
                height: visible ? implicitHeight : 0
                visible: HomeController.editableHasNoMatches
                text: qsTr("No titles match these filters. Try widening the year range, lowering the rating, or using Match Any.")
                color: AppTheme.warning
                font.pixelSize: AppTheme.fs12
                font.weight: Font.DemiBold
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }

            AppButton {
                id: _applyButton
                objectName: "filterApplyButton"

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: _noMatchesText.bottom
                anchors.leftMargin: AppTheme.spacing18
                anchors.rightMargin: AppTheme.spacing18
                anchors.topMargin: AppTheme.spacing12
                height: AppTheme.controlHeightLarge
                text: root.applyText
                accessibleName: root.applyText
                contentRadius: AppTheme.radiusPill
                foregroundColor: AppTheme.darkMode ? AppTheme.onPrimary : "#FFFFFF"

                onClicked: root.applyFilters()
            }
        }
    }
}
