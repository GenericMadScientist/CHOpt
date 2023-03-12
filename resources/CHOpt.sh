#!/bin/sh
# This script is taken from https://doc.qt.io/qt-6/linux-deployment.html and its
# purpose is to let CHOpt find the dynamic libraries included along with it.
appname=$(basename "$0" | sed s,\.sh$,,)

dirname=$(dirname "$0")
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi
LD_LIBRARY_PATH=$dirname/"libs"
export LD_LIBRARY_PATH
"$dirname/$appname" "$@"
