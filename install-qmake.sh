#!/bin/sh

windeployqt.exe ./release/bin/qmidiarp.exe --libdir ./release/bin

lupdate ./qmidiarp.pro
lrelease ./qmidiarp.pro

install -d release/bin/translations
install ./src/translations/*.qm ./release/bin/translations
