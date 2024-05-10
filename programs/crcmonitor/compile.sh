#!/bin/bash

if [[ "$OSTYPE" == "msys" ]]; then
    winpty docker run -v `pwd | sed 's/\//\/\//g'`://src/ -it z88dk/z88dk make $1
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    docker run -v `pwd`:/src/ -it z88dk/z88dk make $1
    python3 ../../scripts/signprg.py MONCRC16.PRG
else
    echo "Unknown operating system"
fi