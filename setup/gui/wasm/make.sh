#!/bin/bash
export EMSDK_QUIET=1
source emsdk/emsdk_env.sh
emcc mat.c -s WASM=1 -s EXPORTED_FUNCTIONS="['_malloc', '_free']" -o mat.js -O1
