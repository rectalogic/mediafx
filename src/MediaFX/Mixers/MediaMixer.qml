// Copyright (C) 2024 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

/*!
    \qmltype MediaMixer
    \inherits Item
    \inqmlmodule MediaFX.Mixers
    \brief Renders a video transition between two \l {MediaClip}s in a \l MediaSequence.
    \sa MediaSequence
*/
Item {
    id: root

    /*! The source item, mixer transitions from \c source to \c dest */
    property Item source
    /*! The destination item, mixer transitions from \c source to \c dest */
    property Item dest
    /*! The duration of the transition in milliseconds */
    property int duration: 1000
    property real time: 0.0

    visible: false

    states: [
        State {
            name: "active"

            ParentChange {
                parent: root
                target: root.source
            }
            PropertyChanges {
                anchors.fill: root
                visible: false
                target: root.source
            }
            ParentChange {
                parent: root
                target: root.dest
            }
            PropertyChanges {
                anchors.fill: root
                visible: false
                target: root.dest
            }
        }
    ]

    onSourceChanged: root.dest ? root.state = "active" : root.state = ""
    onDestChanged: root.source ? root.state = "active" : root.state = ""
}
