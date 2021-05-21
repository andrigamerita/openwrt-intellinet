#!/bin/sh

. ../../define/PATH
PATH=${CROSSDIR}:${PATH}

if [ $# = 1 ]; then
	case "$1" in
		"clean")
			MAKECLEAR=1
			MAKING=0
			;;
		"make")
			MAKECLEAR=0
			MAKING=1
			;;
    esac
fi  
																							
if [ $MAKECLEAR = 1 ]; then
	make clean
    if [ $? != 0 ]; then
        exit 1
    fi						
fi
if [ $MAKING = 1 ]; then
	make clean
	make CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar RANLIB=${CROSS_COMPILE}ranlib BUILD_STRIPPING=y BUILD_NOLIBM=y
#	${CROSS_COMPILE}strip ifrename iwconfig iwevent iwgetid iwlist iwpriv iwspy wpstool

	if [ $? != 0 ]; then
		exit 1
	fi
fi
