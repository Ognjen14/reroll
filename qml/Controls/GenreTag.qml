import QtQuick
import "../Singletons"

Item {
    id: root

    property string text
    property url iconSource: ""
    property real iconSize: 11
    property color backgroundColor: Qt.rgba(AppTheme.primary.r,
                                            AppTheme.primary.g,
                                            AppTheme.primary.b,
                                            0.12)
    property color foregroundColor: AppTheme.primary
    property color borderColor: AppTheme.primary
    property real horizontalPadding: AppTheme.spacing10
    property real verticalPadding: AppTheme.spacing4

    readonly property bool hasIcon: iconSource.toString().length > 0

    implicitWidth: _content.implicitWidth + 2 * horizontalPadding
    implicitHeight: Math.max(_content.implicitHeight, iconSize) + 2 * verticalPadding

    Accessible.role: Accessible.StaticText
    Accessible.name: root.text

    Rectangle {
        anchors.fill: parent
        radius: AppTheme.radiusMedium
        color: root.backgroundColor
        border.width: 1
        border.color: root.borderColor
    }

    Row {
        id: _content

        anchors.centerIn: parent
        spacing: AppTheme.spacing4

        Image {
            width: root.iconSize
            height: root.iconSize
            anchors.verticalCenter: parent.verticalCenter
            visible: root.hasIcon
            source: root.iconSource
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: root.foregroundColor
            font.pixelSize: AppTheme.fs11
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }
    }
}
