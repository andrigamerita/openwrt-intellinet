#!/bin/sh

. ../../target.def
. ../../define/PATH

#ALG_DIR="starcraft"
#ALG_DIR="urlblock"


#ALG_DIR="starcraft msn CPF urlblock"
#ALG_DIR="CPF urlblock"

MODULES_PATH="../module/alg"

mkdir -p modules
mkdir -p $MODULES_PATH

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

for ALG in $ALG_DIR
do
	echo ""
	echo "******************** ALG - [ $ALG ] ********************"
	echo ""

	cd "$ALG"

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
		make CROSS_COMPILE=${CROSS_COMPILE} -j4 -C  ${LINUXDIR} SUBDIRS=../AP/ALG/$ALG modules
		#make
    	if [ $? != 0 ]; then
			echo ""
			echo "******************** complie ALG - $ALG ERROR !! *******************"
			echo ""
	    	#exit 1
		fi
		echo ""
		echo "******************** complie ALG - $ALG SUCCESS !! ********************"
		echo ""
	fi

	cd ..
done

if [ "$MAKING" = "1" ]; then
	echo ""
	echo "******************** Copy modules to $MODULES_PATH ********************"
	echo ""

	for ALG in $ALG_DIR
	do
		echo "copy $ALG modules..."
	
		cd $ALG
		FILE_NAME=${ALG}_mod.ko
		if [ -f $FILE_NAME ]; then
			cp ${ALG}_mod.ko ../$MODULES_PATH
			if [ $? != 0 ]; then
				exit 1
			fi
		else
			echo "$ALG no have ${ALG}_mod.o!!"
		fi
		
		FILE_NAME=${ALG}_nat.o
		if [ -f $FILE_NAME ]; then
			cp ${ALG}_nat.ko ../$MODULES_PATH
			if [ $? != 0 ]; then
				exit 1
			fi
		else
			echo "$ALG no have ${ALG}_nat.o!!"
		fi
		cd ..
	done

#	mv $MODULES_PATH/msn_mod.ko $MODULES_PATH/ip_conntrack_msn.ko
#	mv $MODULES_PATH/msn_nat.ko $MODULES_PATH/ip_nat_msn.ko
#	mv $MODULES_PATH/starcraft_mod.ko $MODULES_PATH/ip_conntrack_starcraft.ko
#	mv $MODULES_PATH/starcraft_nat.ko $MODULES_PATH/ip_nat_starcraft.ko
	
	echo ""
	echo "********** END copy modules ********************"
	echo ""
fi

if [ "$MAKECLEAR" = "1" ]; then
	echo ""
	echo "******************** Remove modules from $MODULES_PATH ********************"
	echo ""

#	rm -rf $MODULES_PATH/*.o
#	rm -rf $MODULES_PATH/*.ko
#	rm -f linux/*.o
#	rm -f linux/*.ko

	echo ""
	echo "******************** END remove modules ********************"
	echo ""
fi




