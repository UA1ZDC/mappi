#!/bin/bash
SRC="$( cd -- "$(dirname "$0")" || exit >/dev/null 2>&1 ; pwd -P )"
export SRC
. $SRC/env-bash
. $SRC/build_tools/cmd_build.sh mappi
