#!/usr/bin/bash

# usage
# ./build.sh build_dir_name install_prefix

# set -x
set -e

BUILD_DIR="build_sh"
INSTALL_PREFIX="Release"

if [[ $# -ge "1" ]]; then
  BUILD_DIR="$1"
fi

if [[ $# == "2" ]]; then
  INSTALL_PREFIX="$2"
fi

echo "# directory: $BUILD_DIR" 1>&2
echo "# install pref: $INSTALL_PREFIX" 1>&2

rm -rvf $BUILD_DIR
mkdir $BUILD_DIR && cd $BUILD_DIR

cmake .. -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX

wait

make

