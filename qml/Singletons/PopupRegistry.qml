pragma Singleton
import QtQuick

QtObject {
    property var _stack: []

    function register(obj) {
        if (!obj)
            return
        var s = _stack.slice()
        if (s.indexOf(obj) === -1) {
            s.push(obj)
            _stack = s
        }
    }

    function unregister(obj) {
        if (!obj)
            return
        var s = _stack.slice()
        var i = s.indexOf(obj)
        if (i !== -1) {
            s.splice(i, 1)
            _stack = s
        }
    }

    function closeTop() {
        while (_stack.length > 0) {
            var obj = _stack[_stack.length - 1]
            if (obj && obj.visible) {
                obj.close()
                return true
            }
            var s = _stack.slice()
            s.pop()
            _stack = s
        }
        return false
    }
}
