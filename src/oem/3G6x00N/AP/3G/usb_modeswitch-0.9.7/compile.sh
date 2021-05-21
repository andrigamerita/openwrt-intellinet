#!/bin/sh
#/opt/buildroot-gdb/bin/mipsel-linux-uclibc-gcc -l usb -o usb_modeswitch usb_modeswitch.c
make
/opt/buildroot-gdb/bin/mipsel-linux-uclibc-strip usb_modeswitch

