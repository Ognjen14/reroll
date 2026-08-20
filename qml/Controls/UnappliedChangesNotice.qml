pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Singletons"

Item {
    id: root

    property bool active: false
    property string messageText: qsTr("You have unapplied changes.")

    visible: opacity > 0
    implicitWidth: 240
    implicitHeight: active ? Math.max(44, _row.implicitHeight + 20) : 0
    opacity: active ? 1 : 0
    clip: true

    Behavior on implicitHeight {
        NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
    }
    Behavior on opacity {
        NumberAnimation { duration: 160 }
    }

    Accessible.role: Accessible.StaticText
    Accessible.name: messageText

    Rectangle {
        anchors.fill: parent
        radius: AppTheme.radiusMedium
        color: AppTheme.warningContainer
        border.width: 1
        border.color: AppTheme.warning

        RowLayout {
            id: _row

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10
            spacing: AppTheme.spacing10

            ThemedIcon {
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
                Layout.alignment: Qt.AlignVCenter
                source: "qrc:/assets/alert-circle.svg"
                tintColor: AppTheme.warning
                showPlaceholder: false
            }

            Text {
                id: _message

                Layout.fillWidth: true
                text: root.messageText
                color: AppTheme.warning
                font.pixelSize: AppTheme.fs13
                font.weight: Font.DemiBold
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }
    }
}
