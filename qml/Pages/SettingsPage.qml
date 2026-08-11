import QtQuick
import QtQuick.Layouts
import "../Controls" as Ctrl
Item{
    id: root
    Flickable {
        anchors.fill:  parent
        contentWidth:  width
        contentHeight: _col.implicitHeight
        clip:          true
        Column {
            id: _col
            width:         parent.width
            topPadding:    24
            bottomPadding: 24
            spacing:       15

            Ctrl.ThemePicker {
                width: parent.width
            }
            Ctrl.AccentPicker {
                width: parent.width
            }
            Ctrl.FontSizePicker{
                width: parent.width
            }
        }
    }
}
