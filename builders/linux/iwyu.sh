#!/usr/bin/env bash

set -e

sudo mkdir -p /mediafx/build/linux && sudo chown mediafx /mediafx/build/ /mediafx/build/linux
cd /mediafx/build/linux
if [ ! -f "qt${QT_VER}.imp" ]; then
    curl -O https://raw.githubusercontent.com/include-what-you-use/include-what-you-use/clang_13/mapgen/iwyu-mapgen-qt.py
    python3 iwyu-mapgen-qt.py /usr/local/Qt/${QT_VER}/gcc_64/include > qt${QT_VER}.imp
fi
CC="clang-13" CXX="clang++-13" cmake -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE="/usr/bin/iwyu;-Xiwyu;--mapping_file=/mediafx/build/linux/qt${QT_VER}.imp;-Xiwyu;--cxx17ns;-Xiwyu;--no_comments" --install-prefix /usr/local/Qt/${QT_VER}/gcc_64 ../.. && cmake --build .