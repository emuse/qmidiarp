#!/bin/sh
echo "Using qmake for qmake build configuration"
echo
touch src/config.h
qmake ./qmidiarp.pro

echo "Done, now run make!"
echo
