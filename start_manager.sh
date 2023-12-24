#!/bin/bash
SRC="/home/mappi/mappi"
export SRC
source "$SRC/env-bash"

$BUILD_DIR/bin/mappi.app.manager
