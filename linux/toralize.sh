#!/bin/env bash

export PROXY_ADDRESS=192.168.0.101
export PROXY_PORT=61298
export LD_PRELOAD=`pwd`/toralize.so

${@}  # execute the command represented as all the arguments
