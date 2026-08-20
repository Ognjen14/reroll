pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import "../Singletons"

AbstractButton {
    id: root

    enum Shape {
        ContentShape,
        SquareShape,
        RoundedSquareShape,
        CircleShape
    }

    property url imageSource
    property bool showImage: imageSource.toString().length > 0
    property int imageSize: 20
    property real contentSpacing: 8
    property bool tintImage: true
    property string imagePlaceholderText: qsTr("IMG")
    property string accessibleName: text
    property int shape: AppButton.ContentShape
    property real shapeSize: 44
    property real contentRadius: 10
    property real roundedSquareRadius: 10

    property color backgroundColor: AppTheme.primary
    property color foregroundColor: AppTheme.onPrimary
    property color disabledBackgroundColor: AppTheme.surfaceVariant
    property color disabledForegroundColor: AppTheme.textDisabled
    property color borderColor: "transparent"
    property alias iconItem: _icon

    readonly property bool hasText: text.length > 0
    readonly property bool hasImageSource: imageSource.toString().length > 0
    readonly property bool hasTextAndImage: hasText && showImage
    readonly property bool showingImagePlaceholder:
        showImage && _icon.placeholderItem.visible
    readonly property bool usesFixedShape: shape !== AppButton.ContentShape
    readonly property real effectiveShapeSize: Math.max(44, shapeSize)
    readonly property real effectiveRadius: {
        const maximumRadius = Math.min(width, height) / 2

        switch (shape) {
        case AppButton.SquareShape:
            return 0
        case AppButton.RoundedSquareShape:
            return Math.min(Math.max(0, roundedSquareRadius), maximumRadius)
        case AppButton.CircleShape:
            return maximumRadius
        default:
            return Math.min(Math.max(0, contentRadius), maximumRadius)
        }
    }

    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    leftPadding: usesFixedShape ? 0 : 16
    rightPadding: usesFixedShape ? 0 : 16
    topPadding: usesFixedShape ? 0 : 10
    bottomPadding: usesFixedShape ? 0 : 10

    implicitWidth: usesFixedShape
                   ? effectiveShapeSize
                   : Math.max(
                         44,
                         leftPadding
                         + (showImage ? imageSize : 0)
                         + (hasTextAndImage ? contentSpacing : 0)
                         + (hasText ? _label.implicitWidth : 0)
                         + rightPadding)
    implicitHeight: usesFixedShape
                    ? effectiveShapeSize
                    : Math.max(
                          44,
                          topPadding
                          + Math.max(showImage ? imageSize : 0,
                                     hasText ? _label.implicitHeight : 0)
                          + bottomPadding)

    Accessible.role: Accessible.Button
    Accessible.name: accessibleName.length > 0 ? accessibleName : qsTr("Button")

    contentItem: Item {
        clip: true

        Row {
            anchors.centerIn: parent
            spacing: root.hasTextAndImage ? root.contentSpacing : 0

            ThemedIcon {
                id: _icon

                width: root.imageSize
                height: root.imageSize
                visible: root.showImage
                source: root.imageSource
                tintEnabled: root.tintImage
                tintColor: root.enabled
                           ? root.foregroundColor
                           : root.disabledForegroundColor
                showPlaceholder: true
                placeholderText: root.imagePlaceholderText
                placeholderBorderColor: root.enabled
                                        ? root.foregroundColor
                                        : root.disabledForegroundColor
                placeholderTextColor: root.enabled
                                      ? root.foregroundColor
                                      : root.disabledForegroundColor
            }

            Text {
                id: _label

                width: Math.min(
                           implicitWidth,
                           Math.max(0,
                                    root.availableWidth
                                    - (root.showImage ? root.imageSize : 0)
                                    - (root.hasTextAndImage
                                       ? root.contentSpacing
                                       : 0)))
                visible: root.hasText
                text: root.text
                color: root.enabled
                       ? root.foregroundColor
                       : root.disabledForegroundColor
                font.pixelSize: AppTheme.fs14
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                maximumLineCount: 1
            }
        }
    }

    background: Rectangle {
        radius: root.effectiveRadius
        color: root.enabled
               ? root.backgroundColor
               : root.disabledBackgroundColor
        border.width: root.visualFocus ? 2 : (root.borderColor.a > 0 ? 1 : 0)
        border.color: root.visualFocus ? AppTheme.focus : root.borderColor

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: root.down
                   ? AppTheme.pressed
                   : (root.hovered ? AppTheme.hover : "transparent")
        }
    }
}
