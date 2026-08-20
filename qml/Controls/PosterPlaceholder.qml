pragma ComponentBehavior: Bound

import QtQuick
import "../Singletons"

Item {
    id: root

    property url source
    property string text: qsTr("POSTER")
    property color backgroundColor: AppTheme.surfaceVariant
    property color borderColor: AppTheme.outline
    property color textColor: AppTheme.textSecondary
    property real radius: 8

    readonly property bool hasSource: source.toString().length > 0
    readonly property bool sourceReady: _placeholderImage.status === Image.Ready
    readonly property bool sourceFailed: hasSource
                                         && _placeholderImage.status === Image.Error

    implicitWidth: 200
    implicitHeight: 300

    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: root.backgroundColor
        border.width: 1
        border.color: root.borderColor
    }

    Image {
        id: _placeholderImage

        anchors.fill: parent
        source: root.source
        sourceSize: Qt.size(
                        Math.max(1, Math.round(width)),
                        Math.max(1, Math.round(height)))
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        cache: true
        smooth: true
        visible: root.sourceReady
    }

    Text {
        anchors.centerIn: parent
        visible: !root.sourceReady
        text: root.text
        color: root.textColor
        font.pixelSize: AppTheme.fs12
        font.weight: Font.DemiBold
    }
}
