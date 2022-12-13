#!/bin/bash

echo "LoadTestingTool starting."

export LD_LIBRARY_PATH="${LOADTOOL}/lib"

bin/loadtestingtool.exe $1