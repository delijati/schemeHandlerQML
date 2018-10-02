import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.4
import QtWebEngine 1.5

Window {
    width: 640
    height: 480
    visible: true
    WebEngineView {
        id: webview
        anchors.fill: parent
        url: "help://www.qt.io"
    }
    Button {
        text: "Enlighten"
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 30
        onClicked: function () {
            console.log("Url change!!")
            webview.url = "bummer://foo.bar"
        }
    }
}
