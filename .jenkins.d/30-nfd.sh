#!/usr/bin/env bash
set -x
set -e

cd /tmp
BUILD="no"
if [ ! -d NFD ]; then
    git clone --recursive --depth 1 git://github.com/named-data/NFD
    cd NFD
    BUILD="yes"
else
    cd NFD
    INSTALLED_VERSION=`git rev-parse HEAD || echo NONE`
    sudo rm -Rf latest-version
    git clone --recursive --depth 1 git://github.com/named-data/NFD latest-version
    cd latest-version
    LATEST_VERSION=`git rev-parse HEAD || echo UNKNOWN`
    cd ..
    rm -Rf latest-version
    if [ "$INSTALLED_VERSION" != "$LATEST_VERSION" ]; then
        cd ..
        sudo rm -Rf NFD
        git clone --recursive --depth 1 git://github.com/named-data/NFD
        cd NFD
        BUILD="yes"
    fi
fi

if [ "$BUILD" = "yes" ]; then
    sudo ./waf -j1 --color=yes distclean
fi

git submodule update --init

./waf configure -j1 --color=yes
./waf -j1 --color=yes
sudo ./waf install -j1 --color=yes
