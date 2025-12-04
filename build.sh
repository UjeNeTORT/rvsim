#!/usr/bin/bash

# usage
# ./build.sh build_dir_name install_prefix

# set -x
set -e

BUILD_DIR="${1:-build_sh}"
INSTALL_PREFIX="${2:-Release}"

echo "# directory: $BUILD_DIR" 1>&2
echo "# install pref: $INSTALL_PREFIX" 1>&2

rm -rvf $BUILD_DIR
mkdir $BUILD_DIR && cd $BUILD_DIR

cmake .. -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
         -DCMAKE_CXX_COMPILER=clang++

wait

make

