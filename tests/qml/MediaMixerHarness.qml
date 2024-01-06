import QtQuick
import QtQuick.Shapes
import QtQuick.Layouts
import QtQuick.Controls

// Invoke using qml tool as "qml MediaMixerHarness.qml -- <fragshader.qsb> [<vertshader.qsb>]
// qsb is specified relative to MediaMixerHarness.qml

Item {
    height: 400
    width: 400

    ColumnLayout {
        anchors.fill: parent

        MediaMixer {
            Layout.fillHeight: true
            Layout.fillWidth: true
            dest: destItem
            fragmentShader: Qt.application.arguments.length >= 4 ? Qt.resolvedUrl(Qt.application.arguments[3]) : ""
            source: sourceItem
            time: time.value
            vertexShader: Qt.application.arguments.length >= 5 ? Qt.resolvedUrl(Qt.application.arguments[4]) : ""
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

        anchors.fill: parent
        color: "lightblue"
        layer.enabled: true
        visible: false

        Shape {
            anchors.fill: parent

            ShapePath {
                fillColor: "orange"
                fillRule: ShapePath.WindingFill
                scale: Qt.size(sourceItem.width / 160, sourceItem.height / 160)
                strokeColor: "darkgray"
                strokeWidth: 3

                PathPolyline {
                    path: [Qt.point(80, 0), Qt.point(130, 160), Qt.point(0, 55), Qt.point(160, 55), Qt.point(30, 160), Qt.point(80, 0),]
                }
            }
        }
    }
    Rectangle {
        id: destItem

        anchors.fill: parent
        color: "yellow"
        layer.enabled: true
        visible: false

        Shape {
            id: qt

            anchors.fill: parent

            ShapePath {
                fillColor: "green"
                scale: Qt.size(destItem.width / 200, destItem.height / 200)
                strokeColor: "darkGray"
                strokeWidth: 3

                PathAngleArc {
                    centerX: 100
                    centerY: 100
                    radiusX: 100
                    radiusY: 100
                    sweepAngle: 360
                }
            }
        }
    }
}
