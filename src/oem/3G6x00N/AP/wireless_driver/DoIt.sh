#!/bin/sh

. ../../target.def
. ../../define/FUNCTION_SCRIPT
. ../../define/COMPILER_CFG.dat

CWD=`pwd`
DIR_LIST="802.1x wsc_upnp2"

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

function check_condition
{
IS_COMPILER="n"
COMPILER_VAR=""
IF_FIND="n"
	if [ "$1" != "" ]; then
		
		for arg in $INDEX
		do
			_DIR=`echo "$arg" | cut -d";" -f 1`
			COMPILER_VAR=""
			if [ "$1" = "$_DIR" ]; then
				COMPILER_VAR=`echo "$arg" | cut -d";" -f 2`
			fi
			if [ "$COMPILER_VAR" != "" ]; then
					IF_FIND="y"
					if [ "$IS_COMPILER" != "y" ]; then
						IS_COMPILER="$COMPILER_VAR"
					fi
			fi
		done
		if [ "$IF_FIND" = "n" ]; then
			IS_COMPILER="y"
		fi

	fi
	
}

for DIR in $DIR_LIST
do
	check_condition "$DIR"
	if [ -d $DIR ] && [ "$IS_COMPILER" != "n" ]; then
		echo ""
		echo "******************** Wireless Driver - [ $DIR ] ********************"
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
		if [ "$DIR" = "ated" ]; then
			make clean
		fi
		if [ "$MAKING" = "1" ]; then
			#Vance 2008.11.10
			if [ "$_UPNP_LIB_VERSION2_" = "y" ]; then 
				if [ "$DIR" = "wsc_upnp2" ]; then
					make clean
					make
					if [ $? != 0 ]; then
					    echo ""
					    echo "******************** complie - $DIR ERROR !! *******************"
					    echo ""
					fi
					cd ..
				else
					make
				fi
			else
				if [ "$DIR" = "wsc_upnp" ]; then
					make clean
					cd libupnp
					make clean
					cd upnp
					make clean
					make
					cd ..
					make
					cd ..
					make
					if [ $? != 0 ]; then
					        echo ""
					    echo "******************** complie - $DIR ERROR !! *******************"
					    echo ""
					fi
					cd ..
				else
					make
				fi
			fi
	
			if [ $? != 0 ]; then
				echo ""
				echo "******************** complie - $DIR ERROR !! *******************"
				echo ""
			else
				echo ""
				echo "******************** complie - $DIR SUCCESS !! ********************"
				echo ""
			fi
		fi
		cd ..
	fi
done
