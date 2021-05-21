#!/bin/sh
. ../../define/PATH
. ../../target.def
. ../../define/FUNCTION_SCRIPT

PATH=${CROSSDIR}:$PATH
CWD=`pwd`
DIR_LIST="libupnp-1.3.1"

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

for DIR in $DIR_LIST
do
	echo ""
	echo "******************** LIBRARY - [ $DIR ] ********************"
	echo ""

	cd "$CWD/""$DIR"

	if [ "$MAKECLEAR" = "1" ]; then
		make clean
	    	if [ $? != 0 ]; then
			echo ""
			echo "******************** make clean ERROR !! ********************"
			echo ""
	    	exit 1
		fi
		echo ""
		echo "******************** make clean SUCCESS !! ********************"
		echo ""
	fi

	if [ "$MAKING" = "1" ]; then
		if [ "$_UPNP_LIB_VERSION2_" = "y" ]; then
			if [ "$DIR" = "libupnp-1.3.1" ]; then
				if [ "$PLATFORM" = "-D_RT305X_" ]; then
					./configure --host=mipsel-linux
				else
					./configure --host=arm-linux
				fi
				cd upnp
				sed -i -e "s/`cat Makefile | grep \"^CFLAGS = \"`/`cat Makefile | grep \"^CFLAGS = \"` \$(FUNCTION)/" Makefile
				sed -i -e "s/# This program is distributed in the hope that it will be useful,/include ..\/..\/..\/..\/define\/FUNCTION_COMPILER/" Makefile
				cd ..
				make clean
				cd ixml; make clean; make CLIENT=0; cd ..
				cd threadutil; make clean; make CLIENT=0; cd ..
				cd upnp; make clean; make CLIENT=0; cd ..
				make

				if [ $? != 0 ]; then
					echo ""
					echo "******************** complie - $DIR ERROR !! *******************"
					echo ""
				fi
			fi
		else
			make
			if [ $? != 0 ]; then
				echo ""
				echo "******************** complie - $DIR ERROR !! *******************"
				echo ""
				exit 1
			fi	
		fi	
			echo ""
			echo "******************** complie - $DIR SUCCESS !! ********************"
			echo ""
	fi

	cd ..
done
