import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
import "./Singletons" as S
import "./Controls" as Ctrl
import "./Pages" as P
import com.topicdev.reroll 1.0
ApplicationWindow {
    id: window

    width:         Qt.platform.os === "windows" ? 1980 : 780
    height:        Qt.platform.os === "windows" ? 1080  : 800
    minimumWidth:  Qt.platform.os === "windows" ? 1360 : 400
    minimumHeight: Qt.platform.os === "windows" ? 720  : 600
    visible: true
    title:   qsTr("Re-Roll")
    color:  S.AppTheme.surface
    // -- Background -----------------------------------
    Rectangle {
        anchors.fill: parent
        color: S.AppTheme.background
        Behavior on color { ColorAnimation { duration: 200 } }
    }

    AndroidSystem{ id: aSys }
    function _applySystemBars(){
        aSys.setLightStatusBar(!S.AppTheme.darkMode)
    }
    function _applyFontScale(){
        S.AppTheme.fontScale = AppSettings.fontSizeScale
    }

    Connections
    {
        target: S.AppTheme
        function onDarkModeChanged() {_applySystemBars()}
    }

    Component.onCompleted:
    {
        _applySystemBars()
        _applyFontScale()


    }


    StackView{
        id: _stack
        anchors.left:   parent.left
        anchors.right:  parent.right
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        initialItem: homePage

        pushEnter:    null
        pushExit:     null
        replaceEnter: null
        replaceExit:  null
        popEnter:     null
        popExit:      null
    }

    Ctrl.NavigationBar{
        id: _navBar
        anchors.left:   parent.left
        anchors.right:  parent.right
        anchors.bottom: parent.bottom
        height: (_stack.depth > 1) ? 0 : implicitHeight
        currentIndex: 0 //Start at default

        onTabSelected: (i)=> {
                            const pages = [homePage,discoverPage,favoritesPage,settingsPage]
                           _stack.replace(pages[i],StackView.Immediate)

        }
    }

    Component{
        id: homePage
        P.HomePage{}
    }

    Component{
        id: discoverPage
        P.DiscoverPage{}
    }
    Component{
        id: favoritesPage
        P.MyList{}
    }

    Component{
        id: settingsPage
        P.SettingsPage{}
    }
}
