import QtQuick 2.15
import "../Singletons"

Item {
    id: root

    property int currentIndex: 0
    signal tabSelected(int index)

    implicitHeight: 64

    function adjustLightness(color, delta) {
        return Qt.hsla(
            color.hslHue,
            color.hslSaturation,
            Math.max(0, Math.min(1, color.hslLightness + delta)),
            color.a
        )
    }
    readonly property color navAccent: AppTheme.darkMode
        ? adjustLightness(AppTheme.primary, 0.0)
        : adjustLightness(AppTheme.primary, 0.0)

    ListModel {
        id: _model
        ListElement { tabId: "home";      label: qsTr("Home")     }
        ListElement { tabId: "discover";  label: qsTr("Discover") }
        ListElement { tabId: "favorites"; label: qsTr("My List")  }
        ListElement { tabId: "taste";     label: qsTr("Taste")    }
        ListElement { tabId: "settings";  label: qsTr("Settings") }
    }

    readonly property real _tabW: root.width / _model.count

    Rectangle {
        id: _bar
        anchors.fill: parent
        color: AppTheme.background
        clip: true
        Behavior on color { ColorAnimation { duration: 200 } }

        Rectangle {
            width: parent.width * 0.95
            anchors.horizontalCenter: parent.horizontalCenter
            height: 1
            color: AppTheme.outline
            Behavior on color { ColorAnimation { duration: 200 } }
        }

        Row {
            anchors.fill: parent
            anchors.topMargin: 12

            Repeater {
                model: _model

                delegate: Item {
                    id: _tab
                    width: _tabW
                    height: parent.height - 12

                    readonly property bool _active: index === root.currentIndex

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.currentIndex = index
                            root.tabSelected(index)
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 6

                        Item {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: 14
                            height: 14

                            Rectangle {
                                anchors.centerIn: parent
                                width: 9
                                height: width
                                radius: width / 2
                                color: _tab._active ? root.navAccent : "transparent"
                                Behavior on color {
                                    ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
                                }
                            }
                        }

                        Text {
                            id: _label
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: model.label
                            font.capitalization: Font.AllUppercase
                            font.letterSpacing: 1
                            font.pixelSize: AppTheme.fs10
                            font.weight: _tab._active ? Font.Bold : Font.Medium
                            color: _tab._active ? root.navAccent : AppTheme.textSecondary
                            scale: _tab._active ? 1.12 : 1.0
                            transformOrigin: Item.Center

                            Behavior on color {
                                ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
                            }
                            Behavior on scale {
                                NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
                            }
                        }

                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: _tab._active ? 18 : 0
                            height: 2
                            radius: 1
                            color: root.navAccent
                            Behavior on width {
                                NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
                            }
                        }
                    }
                }
            }
        }
    }
}
