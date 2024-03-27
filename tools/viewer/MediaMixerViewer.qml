// Copyright (C) 2024 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Shapes
import QtQuick.Layouts
import QtQuick.Controls
import MediaFX
import MediaFX.Mixers

Item {
    id: root

    default property alias data: mixerContainer.data
    required property MediaMixer mixer

    implicitHeight: 400
    implicitWidth: 400
    state: "default"

    states: State {
        name: "default"

        ParentChange {
            parent: mixerContainer
            target: root.mixer
        }
        PropertyChanges {
            dest: destItem
            source: sourceItem
            time: timeSlider.value
            visible: true
            anchors.fill: mixerContainer
            target: root.mixer
        }
    }

    Component.onCompleted: MediaManager.window.color = MediaManager.window.palette.window

    ColumnLayout {
        anchors.fill: parent

        Item {
            id: mixerContainer

            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Slider {
            id: timeSlider

            Layout.fillWidth: true
            from: 0
            to: 1
            value: 0.5
        }
        Button {
            text: "Dump"
            onClicked: root.mixer.dumpItemTree()
        }
    }
    Rectangle {
        id: sourceItem

        color: "lightblue"

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

        color: "yellow"

        Shape {
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
