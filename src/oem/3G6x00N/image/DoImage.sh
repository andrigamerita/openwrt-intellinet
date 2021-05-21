#!/bin/sh

. ../define/PATH
. ../define/FUNCTION_SCRIPT

#-------------------------Build Tester Image ( linux.bin )-------------------------

${CROSS_COMPILE}objcopy -O binary -R .note -R .comment -S vmlinux linux

${ROOTDIR}/toolchain/lzma-4.32.0beta5/src/lzma/lzma -9 -f  -S .lzma linux

objcopy -I binary -O binary --pad-to=0x10FFAC linux.lzma linux-pad.lzma
cat linux-pad.lzma appimg > linux-org.lzma
rm -f linux-pad.lzma

LANG=eng
${ROOTDIR}/toolchain/mkimage -A mips -O linux -T kernel -C lzma -a 80000000 -e $(readelf -h vmlinux | grep "Entry" | awk '{print $4}') -n "Linux Kernel Image"  -d linux-org.lzma linux-org.bin
rm -f linux-org.lzma linux.bin

${GOAHEADDIR}/LINUX/cvimg vmlinux linux-org.bin linux.bin 0x1100000 0x50000
rm -f linux-org.bin

chmod 777 linux.bin
    
mv -f linux.bin ${_MODEL_}_${_MODE_}_${_VERSION_}.bin

ls -l ./${_MODEL_}_${_MODE_}_${_VERSION_}.bin
