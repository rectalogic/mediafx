#!/usr/bin/env bash
# Copyright (C) 2023 Andrew Wason
#
# This file is part of mediaFX.
#
# mediaFX is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# mediaFX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with mediaFX.
# If not, see <https://www.gnu.org/licenses/>.

set -u

CURRENT=${BASH_SOURCE%/*}
source "$CURRENT/../versions"
(
    mkdir -p "${BUILD_ROOT:?}" && cd "$BUILD_ROOT"
    python3 -m venv --clear "build/qtvenv" || exit 1
    "build/qtvenv/bin/pip" install --upgrade --upgrade-strategy eager aqtinstall || exit 1
    "build/qtvenv/bin/python" -m aqt install-qt mac desktop ${QT_VER} --modules qtmultimedia qtquick3d qtshadertools qtquicktimeline qtquickeffectmaker -O "$BUILD_ROOT/installed" || exit 1
    "build/qtvenv/bin/python" -m aqt install-tool mac desktop tools_qtcreator_gui qt.tools.qtcreator_gui -O "$BUILD_ROOT/installed/${QT_VER}/macos/bin" || exit 1
    "build/qtvenv/bin/python" -m aqt install-src mac desktop ${QT_VER} --archives qtbase qtdeclarative qtmultimedia qtquicktimeline -O "$BUILD_ROOT/installed" || exit 1
)
