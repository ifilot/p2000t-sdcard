#!/bin/bash

if [[ "$OSTYPE" == "msys" ]]; then
    winpty docker run -v `pwd | sed 's/\//\/\//g'`://src/ -w //src -it basicmod-compiler make
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    docker run -v `pwd`:/src/ -w /src -it basicmod-compiler make
else
    echo "Unknown operating system"
fi