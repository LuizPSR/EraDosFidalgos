#!/usr/bin/env nix-shell
#!nix-shell -i bash windows.nix
set -e
SRC_DIR=$(realpath $(dirname $0)/..)
cmake --preset windows -S "$SRC_DIR" -B "$SRC_DIR/cmake-build-windows"
cmake --build "$SRC_DIR/cmake-build-windows"
cmake --install "$SRC_DIR/cmake-build-windows" --prefix "$SRC_DIR/windows_install_prefix"
