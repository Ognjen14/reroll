import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
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

    property double _lastBackMs: 0

    // Returns true when the press was consumed (i.e. must NOT close the window).
    function _handleBack() {
        // Some devices deliver both the OnBackInvokedCallback route and a raw
        // KEYCODE_BACK (-> onClosing below) for a single press - without this
        // debounce that would pop/close twice for one press.
        const now = Date.now()
        if (now - _lastBackMs < 250)
            return true
        _lastBackMs = now

        if (S.PopupRegistry.closeTop())
            return true
        if (_stack.depth > 1) {
            _stack.pop()
            return true
        }
        // Root tab page - minimize instead of quitting, so the process stays
        // warm and doesn't look like a crash to the user.
        aSys.minimizeApp()
        return true
    }

    Connections {
        target: aSys
        function onBackPressed() { _handleBack() }
    }

    onClosing: (close) => {
        // Desktop's window-close button should just close the window - only
        // Android routes back through minimize-instead-of-quit.
        if (Qt.platform.os === "android")
            close.accepted = !_handleBack()
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
        anchors.bottom: _navBar.top
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
