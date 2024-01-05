import QtQuick
import QtQuick.Shapes
import QtQuick.Layouts
import QtQuick.Controls

// Invoke using qml tool as "qml MediaMixerHarness.qml -- <fragshader.qsb> [<vertshader.qsb>]
// qsb is specified relative to MediaMixerHarness.qml

Item {
    width: 400
    height: 400
    ColumnLayout {
        anchors.fill: parent
        MediaMixer {
            source: sourceItem
            dest: destItem
            time: time.value
            fragmentShader: Qt.application.arguments.length >= 4 ? Qt.resolvedUrl(Qt.application.arguments[3]) : ""
            vertexShader: Qt.application.arguments.length >= 5 ? Qt.resolvedUrl(Qt.application.arguments[4]) : ""
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        Slider {
            id: time
            Layout.fillWidth: true
            from: 0
            to: 1
            value: 0.5
        }
    }

    Rectangle {
        id: sourceItem
        visible: false
        layer.enabled: true
        anchors.fill: parent
        color: "lightblue"
        Shape {
            anchors.fill: parent
            ShapePath {
                scale: Qt.size(sourceItem.width / 160, sourceItem.height / 160)
                strokeWidth: 3
                strokeColor: "darkgray"
                fillColor: "orange"
                fillRule: ShapePath.WindingFill
                PathPolyline {
                    path: [
                        Qt.point(80, 0),
                        Qt.point(130, 160),
                        Qt.point(0, 55),
                        Qt.point(160, 55),
                        Qt.point(30, 160),
                        Qt.point(80, 0),
                    ]
                }
            }
        }
    }

    Rectangle {
        id: destItem
        visible: false
        layer.enabled: true
        anchors.fill: parent
        color: "yellow"
        Shape {
            id: qt
            anchors.fill: parent
            ShapePath {
                scale: Qt.size(destItem.width / 200, destItem.height / 200)
                strokeWidth: 3
                strokeColor: "darkGray"
                fillColor: "green"
                PathAngleArc { centerX: 100; centerY: 100; radiusX: 100; radiusY: 100; sweepAngle: 360 }         
            }
        }
    }
}