#!/bin/sh
if [ "$1" != "configure" ] && [ "$1" != "clean" ] && [ "$1" != "make" ] && [ "$1" != "install" ] && [ "$1" != "all" ]; then
	echo "$0 [ configure | clean | make | install | all ]"
	exit 1
fi

. ../../define/PATH
PATH=${CROSS_BIN}:$PATH

if [ "$1" = "configure" ] || [ "$1" = "all" ]; then
	:
	if [ $? = 1 ]; then exit 1; fi
fi

if [ "$1" = "clean" ] || [ "$1" = "all" ]; then
	make clean
	if [ $? = 1 ]; then exit 1; fi
fi

if [ "$1" = "make" ] || [ "$1" = "all" ]; then
	make dep
	if [ $? = 1 ]; then exit 1; fi

	make CC=${CROSS_COMPILE}gcc KERNEL_DIR=${LINUXDIR} NO_SHARED_LIBS=1 DO_IPV6=0
	if [ $? = 1 ]; then exit 1; fi

	${CROSS_COMPILE}strip iptables
	if [ $? = 1 ]; then exit 1; fi
fi

if [ "$1" = "install" ] || [ "$1" = "all" ]; then
	mkdir -p ${ROMFS_DIR}/bin
	cp -f iptables ${ROMFS_DIR}/bin
	if [ $? = 1 ]; then exit 1; fi
fi

# Remove:
#   ah addrtype comment connmark conntrack helper length dscp ecn esp hashlimit owner pkttype policy sctp realm tcpmss tos unclean ECN CLASSIFY CONNMARK MIRROR NFQUEUE NOTRACK NETMAP SAME TOS ULOG 
#
# connlimit patch
#   + extensions/.connlimit-test
#   + extensions/libipt_connlimit.c
#   + extensions/libipt_connlimit.man
#
# time patch
#   + extensions/libipt_time.c
#   + extensions/libipt_time.man
#   + extensions/.time-test
#
# webstr patch
#   M extensions/Makefile  (+webstr)
#   + extensions/libipt_webstr.c
#
# Atheros 需要略過錯誤
#   M iptables.c
