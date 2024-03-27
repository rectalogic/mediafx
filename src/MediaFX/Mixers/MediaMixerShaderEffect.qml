import QtQuick

ShaderEffect {
    id: root

    property Item source
    property Item dest
    property real time

    states: [
        State {
            name: "active"

            PropertyChanges {
                layer.enabled: true
                target: root.source
            }
            PropertyChanges {
                layer.enabled: true
                target: root.dest
            }
        }
    ]

    onSourceChanged: root.dest ? root.state = "active" : root.state = ""
    onDestChanged: root.source ? root.state = "active" : root.state = ""
}
