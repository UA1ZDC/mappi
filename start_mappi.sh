#!/bin/bash
SRC="$( cd -- "$(dirname "$0")" || exit >/dev/null 2>&1 ; pwd -P )"
export SRC
source "$SRC/env-bash"

$BUILD_DIR/bin/mappi.mainwindow
