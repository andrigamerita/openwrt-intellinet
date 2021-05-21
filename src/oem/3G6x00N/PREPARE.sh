#!/bin/sh
NOW_PATH=`pwd`
mkdir -p /opt
cd /opt
rm -rf buildroot-gdb
rm -rf buildroot-gcc342
tar jxf ${NOW_PATH}/toolchain/buildroot-gdb-32bit.tar.bz2
tar jxf ${NOW_PATH}/toolchain/buildroot-gcc342.tar.bz2
cd ${NOW_PATH}
