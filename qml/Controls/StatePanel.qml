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

    signal retryRequested()

    implicitWidth: 360
    implicitHeight: _content.implicitHeight + 40
    radius: 16
    color: panelColor
    border.width: 1
    border.color: accentColor

    Accessible.role: Accessible.StaticText
    Accessible.name: titleText
    Accessible.description: messageText

    Column {
        id: _content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 12

        BusyIndicator {
            objectName: "stateBusyIndicator"

            anchors.horizontalCenter: parent.horizontalCenter
            width: 36
            height: visible ? 36 : 0
            visible: root.loading
            running: visible
            Accessible.ignored: true
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

            onClicked: root.retryRequested()
        }
    }
}
