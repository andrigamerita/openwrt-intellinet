#!/bin/sh
. ../../define/PATH

if [ "${APPDIR}" = "" ]; then
	echo "NO Defined PATH!"
	exit 0
fi

rm -f ${APPDIR}/module/alg/*.* 2> /dev/null
rm -f ${APPDIR}/module/led/*.* 2> /dev/null