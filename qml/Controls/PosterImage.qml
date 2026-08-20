pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    property url source
    property size sourceSize: Qt.size(
                                  Math.max(1, Math.round(width)),
                                  Math.max(1, Math.round(height)))
    property int fillMode: Image.PreserveAspectCrop
    property bool cache: true
    property bool smooth: true
    property url placeholderSource: ""
    property string placeholderText: qsTr("POSTER")
    property alias placeholderItem: _placeholder

    readonly property bool hasSource: source.toString().length > 0
    readonly property bool empty: !hasSource
    readonly property bool loading: hasSource && _image.status === Image.Loading
    readonly property bool ready: hasSource && _image.status === Image.Ready
    readonly property bool failed: hasSource && _image.status === Image.Error
    readonly property int imageStatus: _image.status
    readonly property bool showingPlaceholder: !ready

    implicitWidth: 200
    implicitHeight: 300
    clip: true

    Image {
        id: _image

        anchors.fill: parent
        source: root.source
        sourceSize: root.sourceSize
        fillMode: root.fillMode
        asynchronous: true
        cache: root.cache
        smooth: root.smooth
        autoTransform: true
        visible: root.ready
    }

    PosterPlaceholder {
        id: _placeholder

        anchors.fill: parent
        visible: root.showingPlaceholder
        source: root.placeholderSource
        text: root.placeholderText
    }
}
