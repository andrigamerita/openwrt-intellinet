#!/bin/sh
. ../define/PATH
. ../define/FUNCTION_SCRIPT

if [ "$1" = "clean" ]; then
	make V=1 ARCH=mips CROSS_COMPILE=${CROSS_COMPILE}  clean
	exit 1
fi

rm -f include/asm-mips/asm-mips include/asm 
ln -s asm-mips include/asm  

make ARCH=mips CROSS_COMPILE=${CROSS_COMPILE} -j1 modules V=1 
if [ $? != 0 ]; then
    exit 1
fi


cp net/netfilter/nf_conntrack*.ko ${MODDIR}/alg
cp net/ipv4/netfilter/nf_nat*.ko ${MODDIR}/alg
cp net/ipv4/netfilter/ipt_connlimit.ko ${MODDIR}/netfilter
cp -f drivers/net/wireless/rt2860v2_ap/rt2860v2_ap.ko  ${APPDIR}/wireless_driver/module/

if [ "$_DFS_" = "y" ] || [ "$_CARRIER_" = "y" ] ; then
	cp -f arch/mips/rt2880/rt_timer.ko  ${APPDIR}/wireless_driver/module/
fi

make ARCH=mips CROSS_COMPILE=${CROSS_COMPILE} -j1 V=1
if [ $? != 0 ]; then
    exit 1
fi

rm -rf ${IMAGEDIR}/vmlinux
cp ${LINUXDIR}/vmlinux ${IMAGEDIR}
