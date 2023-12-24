#!/bin/bash
SRC="$( cd -- "$(dirname "$0")" || exit >/dev/null 2>&1 ; pwd -P )"
export SRC
source "$SRC/env-bash"

export PATH="/usr/lib/ccache:$PATH" # Override all compiler binaries with ccache
export CCACHE_BASEDIR="$SRC"
export CCACHE_DIR="$HOME/ccache"
export CCACHE_COMPILERCHECK=content # slightly slower than "mtime"

ccache --zero-stats || true
source "$SRC/build_tools/cmd_build.sh" mappi || true
ccache --show-stats

