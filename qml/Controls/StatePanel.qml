pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import "../Singletons"

Rectangle {
    id: root

    enum Mode {
        Loading,
        Empty,
        NetworkError,
        RateLimited
    }

    property int mode: StatePanel.Loading
    readonly property string defaultTitleText: {
        switch (mode) {
        case StatePanel.Empty:
            return qsTr("No suggestions found")
        case StatePanel.NetworkError:
            return qsTr("Unable to load suggestions")
        case StatePanel.RateLimited:
            return qsTr("Please wait before retrying")
        default:
            return qsTr("Loading suggestions")
        }
    }
    readonly property string defaultMessageText: {
        switch (mode) {
        case StatePanel.Empty:
            return qsTr("Try changing your filters to find more titles.")
        case StatePanel.NetworkError:
            return qsTr("Check your connection and try again.")
        case StatePanel.RateLimited:
            return qsTr("The service is temporarily limiting requests.")
        default:
            return qsTr("Finding movies and TV shows for you.")
        }
    }
    property string titleText: defaultTitleText
    property string messageText: defaultMessageText
    property bool retryVisible: false
    property bool retryEnabled: true
    property string retryText: qsTr("Try again")

    readonly property bool loading: mode === StatePanel.Loading
    readonly property color accentColor: {
        switch (mode) {
        case StatePanel.NetworkError:
            return AppTheme.error
        case StatePanel.RateLimited:
            return AppTheme.warning
        default:
            return AppTheme.info
        }
    }
    readonly property color panelColor: {
        switch (mode) {
        case StatePanel.NetworkError:
            return AppTheme.errorContainer
        case StatePanel.RateLimited:
            return AppTheme.warningContainer
        default:
            return AppTheme.surfaceRaised
        }
    }
    readonly property url iconSource: {
        switch (mode) {
        case StatePanel.Empty:
            return "qrc:/assets/nav/discover.svg"
        case StatePanel.NetworkError:
        case StatePanel.RateLimited:
            return "qrc:/assets/alert-circle.svg"
        default:
            return ""
        }
    }

    signal retryRequested()

    implicitWidth: 360
    implicitHeight: _content.implicitHeight + 48
    radius: AppTheme.radiusXLarge
    color: panelColor
    border.width: 1
    border.color: Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.35)

    Accessible.role: Accessible.StaticText
    Accessible.name: titleText
    Accessible.description: messageText

    Column {
        id: _content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        spacing: AppTheme.spacing16

        Item {
            id: _badge

            anchors.horizontalCenter: parent.horizontalCenter
            width: 72
            height: 72

            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.14)
            }

            BusyIndicator {
                objectName: "stateBusyIndicator"

                anchors.centerIn: parent
                width: 34
                height: 34
                visible: root.loading
                running: visible
                Accessible.ignored: true
            }

            ThemedIcon {
                anchors.centerIn: parent
                width: 32
                height: 32
                visible: !root.loading
                source: root.iconSource
                tintColor: root.accentColor
                showPlaceholder: false
            }
        }

        Text {
            width: parent.width
            text: root.titleText
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fs18
            font.weight: Font.Bold
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }

        Text {
            width: parent.width
            visible: text.length > 0
            text: root.messageText
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fs14
            lineHeight: 1.3
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }

        AppButton {
            objectName: "stateRetryButton"

            anchors.horizontalCenter: parent.horizontalCenter
            height: visible ? implicitHeight : 0
            visible: root.retryVisible && !root.loading
            enabled: root.retryEnabled
            text: root.retryText
            accessibleName: root.retryText
            contentRadius: AppTheme.radiusPill
            backgroundColor: root.accentColor
            foregroundColor: "#FFFFFF"

            onClicked: root.retryRequested()
        }
    }
}
