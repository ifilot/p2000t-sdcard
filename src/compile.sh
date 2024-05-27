#!/bin/bash

if [[ "$OSTYPE" == "msys" ]]; then
    winpty docker run -v `pwd | sed 's/\//\/\//g'`://src/ -it z88dk/z88dk make $1
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    docker run -v `pwd`:/src/ -it z88dk/z88dk make $1

    if [[ "$1" == "launcher" ]]; then
        ../../../CPP/crc16sign/build/crc16sign -i LAUNCHER.BIN -o LAUNCHER.BIN -s
    fi

    if [[ "$1" == "" ]]; then
        ../../../CPP/crc16sign/build/crc16sign -i LAUNCHER.BIN -o LAUNCHER.BIN -s
    fi
else
    echo "Unknown operating system"
fi