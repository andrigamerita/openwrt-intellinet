#!/bin/sh

. ../../define/FUNCTION_SCRIPT

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
	"ap")
            if [ "$_AP_WITH_TIMEZONE_" = "y" ]; then
                MAKECLEAR=0
                MAKING=1
            else
                echo "AP don't need this!!"
                exit 0
            fi
			;;
        "ga")
            echo "GA don't need this!!"
            exit 0
            ;;											
    esac
fi																							
. ../../target.def

if [ $MAKECLEAR = 1 ]; then
	rm -f *.o
	rm -f sntpclock
	make clean
fi

if [ $MAKING = 1 ]; then
	echo "$CROSS""gcc -O2" > conf-cc
	echo "$CROSS""gcc -s" > conf-ld
	rm -f *.o
	rm -f sntpclock
	make sntpclock
    	if [ $? != 0 ]; then
	    exit 1
	fi					
fi
