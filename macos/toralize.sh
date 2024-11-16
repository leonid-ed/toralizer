#!/usr/bin/env bash

export SOCKS_VER=5
export PROXY_ADDRESS=127.0.0.1
export PROXY_PORT=62494
export DYLD_INSERT_LIBRARIES=`pwd`/toralize.dylib

${@}  # execute the command represented as all the arguments
