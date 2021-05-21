#!/bin/sh

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
            echo "AP dosn't need this!!"
            exit 0
             ;;
        "ga")
            echo "GA dosn't need this!!"
            exit 0
            ;;
    esac
fi

if [ $MAKECLEAR = 1 ]; then
	#make clean
	if [ $? != 0 ]; then
		exit 1
	fi
fi

if [ $MAKING = 1 ]; then
	cd comgt.0.32
	make clean
	make

	cd ../fping-2.4b2
	make clean
	make

	cd ../option_icon
	make clean
	make

	cd ../ttcp
	make clean
	make

	cd ../usb_modeswitch-0.9.7
	./compile.sh

	cd ..

	if [ $? != 0 ]; then
		exit 1
	fi				
fi

