import QtQuick
import QtQuick.Shapes
import QtQuick.Layouts
import QtQuick.Controls
import MediaFX

Item {
    id: root

    default property alias mixer: mixerContainer.children

    height: 400
    state: "default"
    width: 400

    states: State {
        name: "default"

        PropertyChanges {
            anchors.fill: mixerContainer
            dest: destItem
            source: sourceItem
            target: root.mixer[0]
            time: time.value
        }
    }

    ColumnLayout {
        id: layout

        anchors.fill: parent

        Item {
            id: mixerContainer

            Layout.fillHeight: true
            Layout.fillWidth: true
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
