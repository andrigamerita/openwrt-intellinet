#!/bin/sh

. ../define/PATH
. ../define/FUNCTION_SCRIPT
. ../define/COMPILER_CFG.dat

echo "" > jobs.txt

case "$1" in
        "make")
                export MAKING=1
                export MAKECLEAR=0
                MAKING_IMG=1
                ;;
        "clean")
                export MAKING=0
                export MAKECLEAR=1
                MAKING_IMG=0
                ;;
    *)
        echo "usage $0 [make|clean]"
        exit 1
        ;;
esac


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


	for DIR in $(ls); do
		check_condition "$DIR"
	if [ -d $DIR ] && [ "$IS_COMPILER" != "n" ]; then	
		echo "$DIR" >> jobs.txt
		cd $DIR
		if [ -f DoIt.sh ]; then
			echo ""
			echo "******************** Compiling $DIR ********************"
			echo ""
			if [ "$MAKING_IMG" = "1" ]; then
				./DoIt.sh make
			else
				make clean
				./DoIt.sh clean
			fi
			if [ $? = 0 ]; then
				echo ""
				echo "******************** End compile $DIR ********************"
				echo ""
			else
				echo ""
				echo "******************** compile ERROR in $DIR !! ********************"
				echo ""
				exit 1
			fi
		fi
			cd ..
	fi
done

if [ "$MAKING_IMG" = "1" ]; then
	echo ""
	echo "******************** Collecting Applications ********************"
	echo ""
	
	cd ${APPDIR}/mkimg
	./app_collecting_script.sh
	
	cd ${APPDIR}
	
	echo ""
	echo "******************** Making appimg ********************"
	echo ""
	
	rm -f ${IMAGEDIR}/appimg
	${ROOTDIR}/toolchain/mksquash_lzma-3.2/squashfs3.2-r2/squashfs-tools/mksquashfs ${ROMFSDIR} ${IMAGEDIR}/appimg
	size -A -x --target=binary ${IMAGEDIR}/appimg
	chmod +r ${IMAGEDIR}/appimg
	ls -l ${IMAGEDIR}/appimg
	
	if [ $? = 0 ]; then
		echo ""
		echo "******************** Make appimg OK !! ********************"
		echo ""
	else
		echo ""
		echo "******************** Make appimg FAIL !! ********************"
		echo ""
		exit 1
	fi
fi

	echo ""
	echo "******************** Script finished !! ********************"
	echo ""
